#pragma warning(disable: 4100)
#pragma warning(disable : 4189)
//using std::string;
static float& g_deltaTime = (*(float*)RELOCATION_ID(523660, 410199).address());


namespace hooks
{
	using VM = RE::BSScript::Internal::VirtualMachine;
	using StackID = RE::VMStackID;
#define STATIC_ARGS [[maybe_unused]] VM *a_vm, [[maybe_unused]] StackID a_stackID, RE::StaticFunctionTag *
#define PI 3.14159265358979323846f

	using EventResult = RE::BSEventNotifyControl;

	using tActor_IsMoving = bool (*)(RE::Actor* a_this);
	static REL::Relocation<tActor_IsMoving> IsMoving{ REL::VariantID(36928, 37953, 0x6116C0) };

	using tActor_GetCombatState = RE::ACTOR_COMBAT_STATE (*)(RE::Actor *a_this);
	static REL::Relocation<tActor_GetCombatState> Actor_GetCombatState{REL::VariantID(37603, 38556, 0x62DD00)}; // 624E90, 64A520, 62DD00

	union ConditionParam
	{
		char c;
		std::int32_t i;
		float f;
		RE::TESForm *form;
	};

	bool GetshouldHelp(const RE::Actor *p_ally, const RE::Actor *a_actor);
	bool IsValidLifeState(RE::Actor *a_actor, bool checkDeath = false);
	bool isLastHostileInRange(RE::Actor *attacker, RE::Actor *victim, float range);

	class CombatEventFixes
	{
	public:

		static CombatEventFixes* GetSingleton()
		{
			static CombatEventFixes avInterface;
			return &avInterface;
		}

		static void install();

		static void install_protected(){
			Install_Update();
		}

		static bool BindPapyrusFunctions(VM* vm);
		static void UpdateCombatTarget(RE::Actor* a_actor);
		static bool isInactive(RE::Actor *a_actor);
		void Update(RE::Actor* a_actor, float a_delta);
		int GenerateRandomInt(int value_a, int value_b);
		bool IsCasting(RE::Actor *a_actor);
	    float GenerateRandomFloat(float value_a, float value_b);
		double GenerateRandomDouble(double value_a, double value_b);
		void ClearUpdates(RE::Actor *a_actor, bool clearAll = false);
		void RegisterforUpdate(RE::Actor *a_actor, std::tuple<RE::Actor *, std::chrono::steady_clock::time_point, std::chrono::milliseconds, std::string> data);
		void Process_Updates(RE::Actor *a_actor, std::chrono::steady_clock::time_point time_now);
		static bool GetBoolVariable(RE::Actor *a_actor, std::string a_string);
		static int GetIntVariable(RE::Actor *a_actor, std::string a_string);
		static float GetFloatVariable(RE::Actor *a_actor, std::string a_string);
		void Evaluate_Combat_AI(RE::Actor *a_actor, bool initial = false);

        bool IsCombatDisabled(RE::Actor* a_actor);

        std::shared_mutex mtx_Timer;

		std::unordered_map<RE::Actor *, std::tuple<RE::Actor *, std::chrono::steady_clock::time_point, std::chrono::milliseconds, std::string>> _Timer;

	private:
		CombatEventFixes() = default;
		CombatEventFixes(const CombatEventFixes&) = delete;
		CombatEventFixes(CombatEventFixes&&) = delete;
		~CombatEventFixes() = default;

		CombatEventFixes& operator=(const CombatEventFixes&) = delete;
		CombatEventFixes& operator=(CombatEventFixes&&) = delete;

		std::random_device rd;
		//PRECISION_API::IVPrecision1* _precision_API;
		//static void PrecisionWeaponsCallback_Post(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData& a_hitdata);

	protected:

		struct Actor_Update
		{
			static void thunk(RE::Actor* a_actor, float a_delta)
			{
				func(a_actor, a_delta);
				GetSingleton()->Update(a_actor, g_deltaTime);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		
		static void Install_Update(){
			stl::write_vfunc<RE::Character, 0xAD, Actor_Update>();
		}
		
	};
};

constexpr uint32_t hash(const char* data, size_t const size) noexcept
{
	uint32_t hash = 5381;

	for (const char* c = data; c < data + size; ++c) {
		hash = ((hash << 5) + hash) + (unsigned char)*c;
	}

	return hash;
}

constexpr uint32_t operator"" _h(const char* str, size_t size) noexcept
{
	return hash(str, size);
}
