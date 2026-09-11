#include "hook.h"

namespace hooks
{
	bool IsValidLifeState(RE::Actor *a_actor, bool checkDeath)
	{
		if (checkDeath)
		{
			switch (a_actor->AsActorState()->GetLifeState())
			{
			case RE::ACTOR_LIFE_STATE::kDying:
			case RE::ACTOR_LIFE_STATE::kDead:
				return false;

			default:
				return true;
			}
		}
		else
		{
			switch (a_actor->AsActorState()->GetLifeState())
			{
			case RE::ACTOR_LIFE_STATE::kBleedout:
			case RE::ACTOR_LIFE_STATE::kDying:
			case RE::ACTOR_LIFE_STATE::kDead:
			case RE::ACTOR_LIFE_STATE::kUnconcious:
			case RE::ACTOR_LIFE_STATE::kEssentialDown:
				return false;

			default:
				return true;
			}
		}
	}

    bool CombatEventFixes::IsCombatDisabled(RE::Actor* a_actor) {

        if (GetBoolVariable(a_actor, "IsStaggering") || GetBoolVariable(a_actor, "IsRecoiling") || a_actor->AsActorState()->GetKnockState() != RE::KNOCK_STATE_ENUM::kNormal) 
		{
           return true; 
        }

        return false;
    }

    void CombatEventFixes::UpdateCombatTarget(RE::Actor* a_actor){
		auto CTarget = a_actor->GetActorRuntimeData().currentCombatTarget.get().get();
		if (!CTarget) {
			auto combatGroup = a_actor->GetCombatGroup();
			if (combatGroup) {
				for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
					if (it->targetHandle && it->targetHandle.get().get()) {
						a_actor->GetActorRuntimeData().currentCombatTarget = it->targetHandle;
						break;
					}
					continue;
				}
			}
		}
		//a_actor->UpdateCombat();
	}

	bool CombatEventFixes::isInactive(RE::Actor *a_actor){
		auto result = false;

		if (auto combatcontrol = a_actor->GetActorRuntimeData().combatController; combatcontrol && combatcontrol->inactive)
		{
			result = true;
		}

		return result;
	}


	bool CombatEventFixes::IsCasting(RE::Actor* a_actor)
	{
		bool result = false;

		if ((GetBoolVariable(a_actor, "IsCastingRight")) || (GetBoolVariable(a_actor, "IsCastingLeft")) || (GetBoolVariable(a_actor, "IsCastingDual")))
		{
			result = true;
		}

		return result;
	}

	bool CombatEventFixes::GetBoolVariable(RE::Actor *a_actor, std::string a_string)
	{
		auto result = false;
		a_actor->GetGraphVariableBool(a_string, result);
		return result;
	}

	int CombatEventFixes::GetIntVariable(RE::Actor *a_actor, std::string a_string)
	{
		auto result = 0;
		a_actor->GetGraphVariableInt(a_string, result);
		return result;
	}

	float CombatEventFixes::GetFloatVariable(RE::Actor *a_actor, std::string a_string)
	{
		auto result = 0.0f;
		a_actor->GetGraphVariableFloat(a_string, result);
		return result;
	}


	class OurEventSink :
		public RE::BSTEventSink<RE::TESCombatEvent>,
		public RE::BSTEventSink<RE::TESDeathEvent>
	{
		OurEventSink() = default;
		OurEventSink(const OurEventSink&) = delete;
		OurEventSink(OurEventSink&&) = delete;
		OurEventSink& operator=(const OurEventSink&) = delete;
		OurEventSink& operator=(OurEventSink&&) = delete;

	public:
		static OurEventSink* GetSingleton()
		{
			static OurEventSink singleton;
			return &singleton;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent *event, RE::BSTEventSource<RE::TESDeathEvent> *)
		{
			if (!event || !event->actorDying)
			{
				return RE::BSEventNotifyControl::kContinue;
			}
			const auto a_actor = event->actorDying->As<RE::Actor>();

			if (!a_actor)
			{
				return RE::BSEventNotifyControl::kContinue;
			}

			if (a_actor->IsPlayerRef())
			{
				CombatEventFixes::GetSingleton()->ClearUpdates(a_actor, true);
				
			}
			else
			{
				CombatEventFixes::GetSingleton()->ClearUpdates(a_actor);
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*){
			auto a_actor = event->actor->As<RE::Actor>();

			if (!a_actor || a_actor->IsPlayerRef()) {
				return RE::BSEventNotifyControl::kContinue;
			}

			switch (event->newState.get()) {
			case RE::ACTOR_COMBAT_STATE::kCombat:

				break;

			case RE::ACTOR_COMBAT_STATE::kSearching:

				break;

			case RE::ACTOR_COMBAT_STATE::kNone:
			
				// CombatEventFixes::GetSingleton()->ClearUpdates(a_actor);

				break;

			default:
				break;
			}

			return RE::BSEventNotifyControl::kContinue;
		}
	};
	

	bool GetshouldHelp(const RE::Actor *p_ally, const RE::Actor *a_actor)
	{
		static RE::TESConditionItem cond;
		static std::once_flag flag;
		std::call_once(flag, [&]()
					   {
        cond.data.functionData.function = RE::FUNCTION_DATA::FunctionID::kGetShouldHelp;
        cond.data.flags.opCode          = RE::CONDITION_ITEM_DATA::OpCode::kEqualTo;
        cond.data.comparisonValue.f     = 1.0f; });

		ConditionParam cond_param;
		cond_param.form = const_cast<RE::TESObjectREFR *>(a_actor->As<RE::TESObjectREFR>());
		cond.data.functionData.params[0] = std::bit_cast<void *>(cond_param);

		RE::ConditionCheckParams params(const_cast<RE::TESObjectREFR *>(p_ally->As<RE::TESObjectREFR>()),
										const_cast<RE::TESObjectREFR *>(a_actor->As<RE::TESObjectREFR>()));
		return cond(params);
	}

	void CombatEventFixes::install(){

		auto eventSink = OurEventSink::GetSingleton();

		auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
		// eventSourceHolder->AddEventSink<RE::TESCombatEvent>(eventSink);
		eventSourceHolder->AddEventSink<RE::TESDeathEvent>(eventSink);
	}


	bool CombatEventFixes::BindPapyrusFunctions(VM* vm)
	{
		//vm->RegisterFunction("XXXX", "XXXXX", XXXX);
		return true;
	}

	int CombatEventFixes::GenerateRandomInt(int value_a, int value_b)
	{
		std::mt19937 generator(rd());
		std::uniform_int_distribution<int> dist(value_a, value_b);
		return dist(generator);
	}

	float CombatEventFixes::GenerateRandomFloat(float value_a, float value_b)
	{
		std::mt19937 generator(rd());
		std::uniform_real_distribution<float> dist(value_a, value_b);
		return dist(generator);
	}
	double CombatEventFixes::GenerateRandomDouble(double value_a, double value_b)
	{
		std::mt19937 generator(rd());
		std::uniform_real_distribution<double> dist(value_a, value_b);
		return dist(generator);
	}

	void CombatEventFixes::RegisterforUpdate(RE::Actor *a_actor, std::tuple<RE::Actor*, std::chrono::steady_clock::time_point, std::chrono::milliseconds, std::string> data)
	{
		std::lock_guard<std::shared_mutex> lk(mtx_Timer);
		
		auto itt = _Timer.find(a_actor);
		if (itt == _Timer.end())
		{
			std::vector<std::tuple<RE::Actor *, std::chrono::steady_clock::time_point, std::chrono::milliseconds, std::string>> Hen;
			Hen.emplace_back(data);
			_Timer.emplace(a_actor, Hen);
		}
		else
		{
			itt->second.emplace_back(data);
		}
	}

	void CombatEventFixes::ClearUpdates(RE::Actor *a_actor, bool clearAll)
	{
		std::lock_guard<std::shared_mutex> lk(mtx_Timer);

		if(clearAll)
		{
			_Timer.clear();
			
		}else
		{
			auto itt = _Timer.find(a_actor);
			if (itt != _Timer.end())
			{
				itt->second.clear();
				_Timer.erase(itt);
			}
		}
	}

	void CombatEventFixes::Evaluate_Combat_AI(RE::Actor *a_actor)
	{
		if (!a_actor || !IsValidLifeState(a_actor))
		{
			return;
		}

		if (const auto combatGroup = a_actor->GetCombatGroup(); combatGroup)
		{
			bool isincombat = false;
			for (const auto &targetData : combatGroup->targets)
			{
				if (const auto targetHandle = targetData.targetHandle; targetHandle)
				{
					if (const auto targetPtr = targetData.targetHandle.get(); targetPtr)
					{
						if (const auto target = targetPtr.get(); target)
						{
							if (IsValidLifeState(target, true))
							{
								isincombat = true;
								break;
							}
						}
					}
				}

				continue;
			}
			if (!isincombat)
			{
				logger::info("{} might be stuck in combat. No targets found. Evaluating AI", a_actor->GetName());
				a_actor->EvaluatePackage(true, true);
			}
			else if (isInactive(a_actor))
			{
				logger::info("{} looks inactive in combat. Re-Evaluting AI and targets", a_actor->GetName());
				a_actor->EvaluatePackage(true);
				UpdateCombatTarget(a_actor);
			}
		}
		else
		{
			if (Actor_GetCombatState(a_actor) == RE::ACTOR_COMBAT_STATE::kCombat)
			{
				auto &runtimeData = a_actor->GetActorRuntimeData();
				auto currentTarget = runtimeData.currentCombatTarget.get();
				auto H = CombatEventFixes::GetSingleton();

				if (!(a_actor->IsAttacking() || IsCasting(a_actor)) && (!IsMoving(a_actor) || isInactive(a_actor)))
				{
					if (!currentTarget)
					{
						logger::info("{} might be stuck in combat. {} is not attacking or casting or moving. No combat group found. Evaluting AI", a_actor->GetName(), a_actor->GetName());
						a_actor->EvaluatePackage(true, true);
					}
					else if (isInactive(a_actor))
					{
						logger::info("{} looks inactive in combat. Re-Evaluting AI and targets", a_actor->GetName());
						a_actor->EvaluatePackage(true);
						UpdateCombatTarget(a_actor);
					}
				}
				else if (IsCasting(a_actor) && !IsMoving(a_actor) && !currentTarget)
				{
					logger::info("{} might be stuck in combat. {} is casting but isn't moving and doesn't have a combat group. Evaluting AI", a_actor->GetName(), a_actor->GetName());
					a_actor->EvaluatePackage(true, true);
				}
				else if (!currentTarget)
				{
					logger::info("{} might be stuck in combat. No combat target or combat group found. Evaluting AI", a_actor->GetName());
					a_actor->EvaluatePackage(true, true);
				}
				else if (currentTarget && currentTarget.get() && !IsValidLifeState(currentTarget.get(), true))
				{
					logger::info("{} might be stuck in combat. No combat group found. {} is the combatTarget but is dead. Evaluting AI", a_actor->GetName(), currentTarget.get()->GetName());
					a_actor->EvaluatePackage(true, true);
				}
			}
		}
	}

	void CombatEventFixes::Update(RE::Actor* a_actor, [[maybe_unused]] float a_delta)
	{
		if (a_actor && a_actor->GetActorRuntimeData().currentProcess && a_actor->GetActorRuntimeData().currentProcess->InHighProcess() && a_actor->Is3DLoaded()){

			if (!a_actor || !IsValidLifeState(a_actor))
			{
				return;
			}

			if (Actor_GetCombatState(a_actor) != RE::ACTOR_COMBAT_STATE::kCombat)
			{
				return;
			}

			if (!GetBoolVariable(a_actor, "bPCEF_IsUpdating"))
			{
				auto &runtimeData = a_actor->GetActorRuntimeData();
				auto currentTarget = runtimeData.currentCombatTarget.get();

				if (!(a_actor->IsAttacking() || IsCasting(a_actor)) && (!IsMoving(a_actor) || isInactive(a_actor)))
				{
					if (!currentTarget || isInactive(a_actor))
					{
						a_actor->SetGraphVariableBool("bPCEF_IsUpdating", true);
						RegisterforUpdate(a_actor, std::forward_as_tuple(nullptr, std::chrono::steady_clock::now(), 3000ms, "EvaluateAI_NoTarget_Update"));
					}
				}
				else if (IsCasting(a_actor) && !IsMoving(a_actor) && !currentTarget)
				{
					a_actor->SetGraphVariableBool("bPCEF_IsUpdating", true);
					RegisterforUpdate(a_actor, std::forward_as_tuple(nullptr, std::chrono::steady_clock::now(), 3000ms, "EvaluateAI_NoTarget_Update"));
				}
				else if (!currentTarget)
				{
					a_actor->SetGraphVariableBool("bPCEF_IsUpdating", true);
					RegisterforUpdate(a_actor, std::forward_as_tuple(nullptr, std::chrono::steady_clock::now(), 3000ms, "EvaluateAI_NoTarget_Update"));
				}
				else if (currentTarget && currentTarget.get() && !IsValidLifeState(currentTarget.get(), true))
				{
					a_actor->SetGraphVariableBool("bPCEF_IsUpdating", true);
					RegisterforUpdate(a_actor, std::forward_as_tuple(nullptr, std::chrono::steady_clock::now(), 3000ms, "EvaluateAI_NoTarget_Update"));
				}
			}
			else
			{
				Process_Updates(a_actor, std::chrono::steady_clock::now());
			}
		}
	}
	

	void CombatEventFixes::Process_Updates(RE::Actor *a_actor, std::chrono::steady_clock::time_point time_now)
	{
		if(!a_actor || !IsValidLifeState(a_actor)){
			return;
		}

		std::lock_guard<std::shared_mutex> lk(mtx_Timer);

		auto it = _Timer.find(a_actor);
		if (it != _Timer.end())
		{
			if (!it->second.empty())
			{
				for (auto data : it->second)
				{
					RE::Actor *a_target = nullptr;
					std::chrono::steady_clock::time_point time_initial;
					std::chrono::milliseconds time_required;
					std::string function;
					std::tie(a_target, time_initial, time_required, function) = data;

					if (duration_cast<std::chrono::milliseconds>(time_now - time_initial).count() >= time_required.count())
					{
						auto H = RE::TESDataHandler::GetSingleton();
						switch (hash(function.c_str(), function.size()))
						{
						case "EvaluateAI_NoTarget_Update"_h:
							Evaluate_Combat_AI(a_actor);
							a_actor->SetGraphVariableBool("bPCEF_IsUpdating", false);
							break;

						default:
							break;
						}
						std::vector<std::tuple<RE::Actor *, std::chrono::steady_clock::time_point, std::chrono::milliseconds, std::string>>::iterator position = std::find(it->second.begin(), it->second.end(), data);
						if (position != it->second.end())
						{
							it->second.erase(position);
						}
					}
				}
			}
			else
			{
				_Timer.erase(it);
			}
		}
	}
}
