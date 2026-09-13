#pragma once

#include "RE/A/Actor.h"
#include "RE/A/ActorValues.h"
#include "RE/T/TESForm.h"
#include "REL/Relocation.h"
#include "REX/W32/KERNEL32.h"

#include <cassert>
#include <span>
#include <string_view>

#ifndef RELOCATION_ID
#error "API_ActorValueGenerator.h requires the macro RELOCATION_ID to be defined."
#endif

#ifdef AVG_ENABLE_LOGGING
#include "REX/LOG.h"
#endif


#ifdef UNICODE
#define AVG_API_SOURCE L"ActorValueGenerator.dll"
#define GET_MODULE_HANDLE REX::W32::GetModuleHandleW
#else
#define AVG_API_SOURCE "ActorValueGenerator.dll"
#define GET_MODULE_HANDLE REX::W32::GetModuleHandleA
#endif

namespace RE
{
    //If there is ever some detail of general AV usage that needs access (GetDelay, SetDelay, etc) I will place them here.
    // The prefered should be whatever clib uses, but since they might not always be ported I'll put them here.
    namespace detail
    {
        inline RE::ActorValue GetActorValueIDFromName(const char* av_name)
        {
            //SE: 0x3E1450, AE: 0x3FC5A0, VR: ---
            using func_t = decltype(&GetActorValueIDFromName);
            REL::Relocation<func_t> func{ REL::RelocationID(26570, 27203) };
            return func(av_name);
        }
    }

    inline const char* GetActorValueName(RE::ActorValue av)
    {
        //SE: 3E1230, AE: 3FC360, VR: ---
        using func_t = decltype(&GetActorValueName);
        REL::Relocation<func_t> func{ REL::RelocationID(26563, 27195) };
        return func(av);
    }

    inline const char* GetActorValueScriptName(RE::ActorValue av)
    {
        //SE: 3E1130, AE: 3FC250, VR: ---
        using func_t = decltype(&GetActorValueScriptName);
        REL::Relocation<func_t> func{ REL::RelocationID(26561, 27192) };
        return func(av);
    }

    inline RE::ActorValue GetActorValueIDFromName(std::string_view av_name)
    {
        //Please make sure that you are using a null terminated string view. Wouldn't want it to be completely incorrect.
        return detail::GetActorValueIDFromName(av_name.data());
    }
}

namespace AVG
{
#define ACTOR_VALUE_CHANGE_PARAMS RE::Actor* target, RE::Actor* cause, RE::ActorValue av, RE::ACTOR_VALUE_MODIFIER modifier, float before, float after

    using ActorValueChange = void(*)(ACTOR_VALUE_CHANGE_PARAMS);

    using GetAVDelegate = float(*)(std::string_view, std::span<RE::ACTOR_VALUE_MODIFIER>, RE::Actor*);
    using SetAVDelegate = void(*)(std::string_view, RE::ACTOR_VALUE_MODIFIER, RE::Actor*, RE::Actor*, float, float);

    namespace detail
    {
        enum class LogLevel
        {
            Trace,
            Debug,
            Info,
            Warn,
            Error,
            Critical
        };

#ifdef AVG_ENABLE_LOGGING
        inline static constexpr bool doLog = true;
        template <LogLevel Level>
        inline void Log(std::string_view message)
        {
            if constexpr (Level == LogLevel::Trace) {
                REX::TRACE(message);
            }
            else if constexpr (Level == LogLevel::Debug) {
                REX::DEBUG(message);
            }
            else if constexpr (Level == LogLevel::Info) {
                REX::INFO(message);
            }
            else if constexpr (Level == LogLevel::Warn) {
                REX::WARN(message);
            }
            else if constexpr (Level == LogLevel::Error) {
                REX::ERROR(message);
            }
            else if constexpr (Level == LogLevel::Critical) {
                REX::CRITICAL(message);
            }
        }
#else
        inline static constexpr bool doLog = false;
        template <LogLevel>
        inline constexpr void Log(std::string_view)
        {
        }
#endif
    }

    namespace API
    {
        enum struct DelegateResult
        {
            Success,
            Nonexistent,	//The name of the extra value doesn't exist
            Nonfunctional,	//The extra value isn't a functional value
            Nondelegate,	//The functional value hasn't been marked as a delegate yet.
            AlreadyFilled,	//The delegate values are already filled.
            Unknown,

        };
        enum Version
        {
            Version1,
            Version2,
            Version3,

            Current = Version3
        };

        struct InterfaceVersion1
        {
            inline static constexpr auto VERSION = Version::Version1;

            virtual ~InterfaceVersion1() = default;

            /// <summary>
            /// Gets the current version of the interface.
            /// </summary>
            /// <returns></returns>
            [[nodiscard]] virtual Version GetVersion() = 0;

            /// <summary>
            /// Resolves an ExtraValue after a save game has been loaded.
            /// </summary>
            /// <param name="av_ref">ExtraValue to resolve, treated as an actor value.</param>
            /// <returns></returns>
            [[nodiscard]] virtual RE::ActorValue ResolveExtraValue(RE::ActorValue av_ref) = 0;
        };

        struct InterfaceVersion2 : public InterfaceVersion1
        {
            inline static constexpr auto VERSION = Version::Version2;

            /// <summary>
            /// Registers a function to fire after an ActorValue changes it's base or modifier values. Function requires parameters:
            /// <para>  RE::Actor* target: the target of the change</para> 
            /// <para>	RE::Actor* cause: cause of the change (available only on damage)</para> 
            /// <para>	RE::ActorValue av: The Changing actor value</para> 
            /// <para>	RE::ACTOR_VALUE_MODIFIER modifier: The modifier impacted (kTotal is the base value)</para> 
            /// <para>	float prev_value: The value before the change happened</para> 
            /// <para>	float new_value: The value before the change happened</para> 
            /// <para>	bool recursive: A check if this event is firing off within another event</para> 
            /// <para>	*Use macro ACTOR_VALUE_CHANGE_PARAMS to set up parameters as above automatically </para> 
            /// </summary>
            virtual void RegisterForActorValueChange(ActorValueChange func) = 0;
        };

        struct InterfaceVersion3 : public InterfaceVersion2
        {
            inline static constexpr auto VERSION = Version::Version3;

            /// <summary>
            /// Registers a function to be used as a functional value's formula. Returns false
            /// </summary>
            /// <param name="get"></param>
            /// <param name="set"></param>
            /// <returns></returns>
            virtual DelegateResult RegisterAVDelegate(std::string_view name, GetAVDelegate get, SetAVDelegate set = nullptr) = 0;

            /// <summary>
            /// Processes dynamic form's aliases, will ignore plugin and form list
            /// </summary>
            /// <param name="form">form to process</param>
            /// <returns></returns>
            virtual int64_t ProcessFormAliases([[maybe_unused]]RE::TESForm* form) {
                return -1;
            }
        };

        using CurrentInterface = InterfaceVersion3;

        /// <summary>
        /// Accesses the ActorValueGenerator Interface. Using the template version is advised. Safe to call PostLoad
        /// </summary>
        /// <param name="version"> to request.</param>
        /// <returns>Returns void* of the interface, cast to the respective version.</returns>
        inline void* RequestInterface(Version version)
        {
            typedef void* (__stdcall* RequestFunction)(Version);
            const auto api = GET_MODULE_HANDLE(AVG_API_SOURCE);

            if (api == nullptr) {
                detail::Log<detail::LogLevel::Warn>("ActorValueGenerator.dll not found, API will remain non functional.");
                return nullptr;
            }

            const auto request_interface =
                reinterpret_cast<RequestFunction>(
                    REX::W32::GetProcAddress(api, "AVG_RequestInterfaceImpl")
                    );

            if (!request_interface) {
                detail::Log<detail::LogLevel::Critical>("AVG request interface not found");
                return nullptr;
            }

            return request_interface(version);
        }

        /// <summary>
        /// Accesses the ActorValueGenerator Interface, safe to call PostLoad
        /// </summary>
        /// <typeparam name="InterfaceClass">is the class derived from the interface to use.</typeparam>
        /// <returns>Casts to and returns a specific version of the interface.</returns>
        template <class InterfaceClass = CurrentInterface>
        inline  InterfaceClass* RequestInterface(bool required = true)
        {
            static InterfaceClass* intfc = nullptr;

            if (!intfc) {
                intfc = reinterpret_cast<InterfaceClass*>(RequestInterface(InterfaceClass::VERSION));
                if (required)
                    assert(intfc);
            }

            return intfc;
        }
    }

    struct ExtraValue
    {
        //It is in one's best interest to NEVER statically create these. Needless to say it wouldn't wait to see if AVG exists, and would get none

        constexpr ExtraValue() = default;

        constexpr ExtraValue(RE::ActorValue a) : _av{ a != RE::ActorValue::kTotal ? a : RE::ActorValue::kNone } {}

        ExtraValue(std::string_view name) : _name{ name }, _av{ RE::ActorValue::kTotal } {}

        RE::ActorValue Resolve() const
        {
            if (auto api = API::RequestInterface(); api) {
                _av = api->ResolveExtraValue(_av);
            }
            else if (_av > RE::ActorValue::kTotal) {
                _av = RE::ActorValue::kNone;
            }

            return _av;
        }

        RE::ActorValue get() const
        {
            if (_av == RE::ActorValue::kTotal) {
                auto api = API::RequestInterface(false);

                assert(api);

                if (!api) {
                    return RE::ActorValue::kNone;
                }

                _av = RE::GetActorValueIDFromName(_name);
            }

            assert(_av != RE::ActorValue::kNone);
            
            return _av;
        }

        operator RE::ActorValue() const
        {
            return get();
        }

    private:
        mutable RE::ActorValue _av = RE::ActorValue::kNone;
        std::string_view _name;
    };
}