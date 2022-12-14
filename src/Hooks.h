#pragma once

#include "ExtraValueInfo.h"
#include "ExtraValueStorage.h"
#include "Utility.h"
#include "xbyak/xbyak.h"

namespace AVG
{
	
	//General note, you don't need to be putting the xbyak code on the trampoline, it's effectively exclusively for personal use,
	// and we're jumping from it locally.


	struct GetActorValueHook
	{
		static void Patch()
		{
			//Use variantID at some point pls.
			auto hook_addr = REL::ID(37517 /*0x620D60*/).address();
			auto return_addr = hook_addr + 0x6;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//Preserves these instructions, then jumps to the last functional instruction.
					push(rbp);
					push(rsi);
					push(rdi);
					push(r14);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			func = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk);
			//*/
		}

		static float thunk(RE::ActorValueOwner* a_this, RE::ActorValue av)
		{
			RE::Character* target = skyrim_cast<RE::Character*>(a_this);

			uint32_t raw_value = std::bit_cast<uint32_t>(av);

			
			if (a_this->GetIsPlayerOwner() == true && std::bit_cast<uint32_t>(RE::ActorValue::kVariable01) == raw_value) {
				int catcher = 4;
				logger::info("catcher {} and av {}", catcher, raw_value);
			}

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);


			if (info) //raw_value == 256
			{
				auto value = info->GetExtraValue(target, ExtraValueInput::All);
				//auto value = Psuedo::GetExtraValue(target, "HitsTaken");
				logger::info("hit get {}, VALUE: {}", raw_value, value);
				return value;
				
			}
			else 
			{
				//logger::info("pass get {}", raw_value);
				return func(a_this, av);
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct SetActorValueHook
	{
		//Note, hook is actually for SetBaseActorValue. There's a sub under this that effectively is only used by this.
		// If you anticipate problems hook that instead, if not keep hooking this.

		static void Patch()
		{
			//Use variantID at some point pls.
			auto hook_addr = REL::ID(37520 /*0x621070*/).address();
			auto return_addr = hook_addr + 0x7;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//Preserves these instructions, then jumps to the last functional instruction.
					sub(rsp, 0x38);
					cmp(edx, 0x0FFFFFFFF);
					

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			func = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk);
			//*/
		}

		static void thunk(RE::ActorValueOwner* a_this, RE::ActorValue av, float value)
		{
			RE::Character* target = skyrim_cast<RE::Character*>(a_this);

			uint32_t raw_value = std::bit_cast<uint32_t>(av);

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);

			if (info){//raw_value == 256) {
				logger::info("hit set {}", raw_value);
				info->SetExtraValue(target, value, RE::ACTOR_VALUE_MODIFIER::kTotal);
				//Psuedo::SetExtraValue(target, "HitsTaken", value);
			} else {
				//logger::info("pass set {}", raw_value);
				return func(a_this, av, value);
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	
	struct ModActorValueHook
	{
		//
		//Note, hook is actually for SetBaseActorValue

		static void Patch()
		{
			//Use variantID at some point pls.
			auto hook_addr = REL::ID(37523 /*0x621120*/).address();
			auto return_addr = hook_addr + 0x5;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//uintptr_t arg_8 = qword ptr 10h
					mov(ptr[rsp + 0x10], rbx);
					
					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			func = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk);
			//*/
		}

		static void thunk(RE::Character* a_this, RE::ACTOR_VALUE_MODIFIER a2, RE::ActorValue a3, float a4, RE::Character* actor)
		{
			uint32_t raw_value = std::bit_cast<uint32_t>(a3);

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);

			if (info) {  //raw_value == 256) {
				logger::info("hit mod {}, {}, val {}, who {}", raw_value, (int32_t)a2, a4, !actor ? "none" : actor->GetName());
				info->ModExtraValue(a_this, a4, a2);
				//Psuedo::ModExtraValue(a_this, "HitsTaken", a4, a2);
			} else {
				//logger::info("pass mod {}, {}, val {}, who {}", raw_value, (int32_t)a2, a4, !actor ? "none" : actor->GetName());
				return func(a_this, a2, a3, a4, actor);
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	struct GetBaseActorValueHook
	{
		//
		//Note, hook is actually for SetBaseActorValue

		static void Patch()
		{
			//Use variantID at some point pls.
			auto hook_addr = REL::ID(37519 /*0x620F30*/).address();
			auto return_addr = hook_addr + 0x5;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//uintptr_t arg_10 = qword ptr 18h;
					mov(ptr[rsp + 0x18], rbx);
					
					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			func = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk);
			//*/
		}

		static float thunk(RE::ActorValueOwner* a_this, RE::ActorValue a2)
		{
			RE::Character* target = skyrim_cast<RE::Character*>(a_this);

			uint32_t raw_value = std::bit_cast<uint32_t>(a2);

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);

			if (info) {  //raw_value == 256) {
				auto value = info->GetExtraValue(target, ExtraValueInput::Base);
				//auto value = Psuedo::GetExtraValue(target, "HitsTaken", ExtraValueInput::Base);
				logger::info("hit base {}, val {}", raw_value, value);

				return value;
			} 
			else 
			{
				auto value = func(a_this, a2);
				//logger::info("pass base {}, val {}", raw_value, value);
				
				return value;
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	
	struct GetActorValueModifierHook
	{
		//
		//Note, hook is actually for SetBaseActorValue

		static void Patch()
		{
			//Use variantID at some point pls.
			auto hook_addr = REL::ID(37524 /*0x621350*/).address();
			auto return_addr = hook_addr + 0x6;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					push(rbx);
					sub(rsp, 0x20);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			func = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk);
			//*/
		}

		static float thunk(RE::Character* a_this, RE::ACTOR_VALUE_MODIFIER a2, RE::ActorValue a3)
		{
			uint32_t raw_value = std::bit_cast<uint32_t>(a3);

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);

			if (info) {  //raw_value == 256) {
				//auto value = Psuedo::GetExtraValue(a_this, "HitsTaken", a2);
				auto value = info->GetExtraValue(a_this, a2);
				logger::info("hit mdfr {}, modifier {}, val {}", raw_value, a2, value);

				return value;
			} 
			else 
			{
				//return func(a_this, a2, a3);
				auto value = func(a_this, a2, a3);
				//logger::info("pass mdfr {}, modifier {}, val {}", raw_value, a2, value);

				return value;
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	
	struct GetActorValueIDFromNameHook
	{
		static void Patch()
		{
			//Use variantID at some point pls.
			auto hook_addr = REL::ID(26570 /*0x3E1450*/).address();
			auto return_addr = hook_addr + 0x5;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//uintptr_t arg_0 = qword ptr 8;
					mov(ptr[rsp + 0x8], rbx);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
		
			auto& trampoline = SKSE::GetTrampoline();

			func = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk);
			
			//*/
		}

		static RE::ActorValue thunk(char* av_name)
		{
			//Would like strcmp with case insensitivity
			/*
			int i = 0;

			while ()
			{
				char& letter = av_name
			}
			//*/

			
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(av_name);

			if (info) {//Utility::StrCmpI(av_name, "HitsTaken") == true) {
				ValueID id = info->GetValueID();
				logger::info("EV Queried at  {}", id);
				return static_cast<RE::ActorValue>(id);
			}
			else {
				return func(av_name);
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct temp_MainUpdateHook
	{
		//I hate this method of putting stuff on the main thread like this
		static void Patch()
		{
			auto& trampoline = SKSE::GetTrampoline();
			
			REL::Relocation<uintptr_t> mainHook{ REL::ID(35551) };  // 5AF3D0, courtesy of TDM from Ersh

			func = trampoline.write_call<5>(mainHook.address() + 0x11F, thunk);
			
		}



		static void thunk(RE::Main* a_this, float a2)
		{
			//I'm just going to call this shit fucked for now and call it a day.
			func(a_this, a2);
			
			auto a_ui = RE::UI::GetSingleton();


			if (!a_ui || a_ui->numPausesGame || a_this->freezeTime || !a_this->gameActive) {
				return;
			}
			
			a2 = Utility::GetDeltaTime();

			static bool first = false;

			if (!first) {
				logger::info("{}", a2);
				first = true;
			}
			auto* player = RE::PlayerCharacter::GetSingleton();

			if (temp_player_delay > 0) {

				float remainder = a2 - temp_player_delay;
				temp_player_delay -= a2;
				a2 = fmax(remainder, 0);

				if (a2 > 0) {
					logger::info("player is regenerating hits taken");
				}
			}
			
			if (a2 > 0) 
			{
				float mod_value = 5 * a2;
				Psuedo::ModExtraValue(player, "HitsTaken", mod_value, RE::ACTOR_VALUE_MODIFIER::kDamage);
			}
			
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	
	struct ActorUpdateHook
	{
		//I hate this method of putting stuff on the main thread like this
		static void Patch()
		{
			REL::Relocation<uintptr_t> Character__Actor_VTable{ RE::VTABLE_Character[0] };
			REL::Relocation<uintptr_t> PlayerCharacter__Actor_VTable{ RE::VTABLE_PlayerCharacter[0] };

			func[0] = Character__Actor_VTable.write_vfunc(0xAD, thunk<0>);
			func[1] = PlayerCharacter__Actor_VTable.write_vfunc(0xAD, thunk<1>);
		}
		//The main purpose of this function shouldn't be to to update directly, I was thinking it should be to update states primarily?
		template <unsigned int I = 0>
		static void thunk(RE::Character* a_this, float a2)
		{
			func[I](a_this, a2);
			
			if (a2 == 0)
				a2 = Utility::GetDeltaTime();

			//A question remains if I would like to handle this by recording the last time it was updated, instead of just respecting
			// delta times alone. I'll decide later.

			//Note, this is not how this shit would be going down.
			
			//This would never create, if it doesn't exist, no damage can exist.
			ExtraValueStorage* value_storage = ExtraValueStorage::GetStorage(a_this);

			if (!value_storage || value_storage->GetValue(0, ExtraValueInput::Damage) >= 0) {
				//if (a_this->IsPlayerRef())
				//	logger::info("A {}", value_storage != nullptr);
				
				return;
			}
			//reduce using this
			float remainder = a2 - value_storage->_recoveryData[0].second;

			if (remainder <= 0) {
				value_storage->_recoveryData[0].second -= a2;

				//if (a_this->IsPlayerRef())
				//	logger::info("B {}", remainder);

				return;
			}

			value_storage->_recoveryData[0].second = 0;

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByData(0);

			if (!info)
				return;

			RecoveryData* rec_data = info->GetRecoveryData();

			if (!rec_data)
				return;


			float mod_value = rec_data->tmp_recRate *  remainder;

			//if (a_this->IsPlayerRef())
			//	logger::info("C {}", remainder);

			Psuedo::ModExtraValue(a_this, "HitsTaken", mod_value, RE::ACTOR_VALUE_MODIFIER::kDamage);
						
		}

		static inline REL::Relocation<decltype(thunk<0>)> func[2];
	};


	struct Hooks
	{
		static void Install()
		{
			//Simple safe measure, I don't need the space atm though.
			SKSE::AllocTrampoline(256);


			//Remember to allocate the trampoline smile.

			//Here's a thought, instead of patching this shit manually, what would happen if I say, just pluck the function from it's
			// vtable to get where I'm supposed to pull from? Quite devilish, and no fox hunts. Though I would have to test for 
			// proper preservation.
			// 
			//REL::Relocation<uintptr_t> Character__ActorValueOwner_VTable{ RE::VTABLE_Character[6] };

			//temp_MainUpdateHook::Patch();
			ActorUpdateHook::Patch();

			GetActorValueHook::Patch();
			SetActorValueHook::Patch();
			ModActorValueHook::Patch();
			GetBaseActorValueHook::Patch();
			GetActorValueModifierHook::Patch();
			GetActorValueIDFromNameHook::Patch();


			constexpr std::uint8_t NoOperation3[0x3]{ 0x0F, 0x1F, 0x00 };
			static_assert(sizeof(NoOperation3) == 0x3);
			
			auto comp_id1 = REL::ID(34269 /*0x5671F0*/);
			auto comp_id2 = REL::ID(34271 /*0x5672C0*/);
			
			REL::safe_write(comp_id1.address() + 0x3E, &NoOperation3, 0x3);
			REL::safe_write(comp_id2.address() + 0xCD, &NoOperation3, 0x3);

			//GetActorValueHook::func = Character__ActorValueOwner_VTable.write_vfunc(0x01, GetActorValueHook::thunk);
			//SetActorValueHook::func = Character__ActorValueOwner_VTable.write_vfunc(0x07, SetActorValueHook::thunk);
		}
	};
	
}

//GetActorValue, handled. A collective function.
//GetPermanentActorValue(GetMaximumActorValue), not yet handled. Is actually get maximum value. Name is erroneous.
//GetBaseActorValue, non-handled. Straight forward to do so.
//SetBaseActorValue, handled.
//ModActorValue(ModifyActorValue), non-handled, confused to what it actually does, name may be misleading.
//RestoreActorValue(RestoreActorValue), hook complete. Handled via 140621120. There is a non-vtable version of this. Seems to iterface well enough.
//SetActorValue, basically is SetBaseActorValue

//CheckClampDamageModifier, needs spot treating, but otherwise you know. Won't encounter for the most part because of effects. But yeah.
//GetActorValueModifier(140621350) Needs a direct hook