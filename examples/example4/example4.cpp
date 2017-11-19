#include <iostream>
#include <string>

#include <kangaru/kangaru.hpp>

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers overriding services
 */

using std::cout;
using std::endl;

struct Wand {
	virtual void doTrick() = 0;
};

struct MagicWand : Wand {
	void doTrick() override {
		cout << "It's doing magic tricks!" << endl;
	}
};

struct FireWand : MagicWand {
	void doTrick() override {
		cout << "It's doing fire tricks!" << endl;
	}
};

struct LavaWand : FireWand {
	void doTrick() override {
		cout << "It's doing lava tricks!" << endl;
	}
};

struct Trickster {
	Trickster(Wand& _wand) : wand{_wand} {}
	
	void doTrick() {
		wand.doTrick();
	}
	
private:
	Wand& wand;
};

struct Wizard {
	Wizard(MagicWand& _wand) : wand{_wand} {}
	
	void doTrick() {
		wand.doTrick();
	}
	
private:
	MagicWand& wand;
};

struct FireMage {
	FireMage(FireWand& _wand) : wand{_wand} {}
	
	void doTrick() {
		wand.doTrick();
	}
	
private:
	FireWand& wand;
};

struct MagicWandService;

struct WandService : kgr::abstract_service<Wand>, kgr::defaults_to<MagicWandService> {};

struct MagicWandService : kgr::single_service<MagicWand>, kgr::overrides<WandService> {};
struct FireWandService : kgr::single_service<FireWand>, kgr::overrides<MagicWandService> {};
struct LavaWandService : kgr::single_service<LavaWand>, kgr::overrides<FireWandService, MagicWandService> {};

struct TricksterService : kgr::service<Trickster, kgr::dependency<WandService>> {};
struct WizardService : kgr::service<Wizard, kgr::dependency<MagicWandService>> {};
struct FireMageService : kgr::service<FireMage, kgr::dependency<FireWandService>> {};

int main()
{
	kgr::container container;
	
	// Here, because of the default service type of WandService, MagicWandService is chosen.
	// MagicWand is the first, because it's the highest non-abstract service in the hierarchy.
	// If WandService didn't had that default service type, it would be a runtime error.
	container.service<WandService>();
	
	// FireWand is the second, because it's the second service in the hierarchy.
	container.service<FireWandService>();
	
	// LavaWand is the last, because it's the last service in the hierarchy.
	container.service<LavaWandService>();
	
	auto trickster = container.service<TricksterService>();
	auto wizard = container.service<WizardService>();
	auto fireMage = container.service<FireMageService>();
	
	// The trickster will show "It's doing magic tricks!"
	// because the only service that overrides Wand is MagicWand.
	// Even if another service is overriding MagicWand, it does not overrides Wand.
	trickster.doTrick();
	
	// The trickster will show "It's doing lava tricks!"
	// because LavaWand overrides MagicWand, which was the Wizard's dependency.
	// Even if FireWand is overriding MagicWand, LavaWand is lower in the hierarchy,
	// which grants it priority.
	// A misconfigured hierarchy may lead to incorrect result. Invert instance calls and see by yourself.
	wizard.doTrick();
	
	// The trickster will show "It's doing lava tricks!"
	// because LavaWand is overriding FireWand.
	fireMage.doTrick();
}
