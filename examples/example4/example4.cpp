#include <iostream>
#include <string>
#include <memory>

#include "kangaru.hpp"

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers overriding services
 */

using namespace std;
using namespace kgr;

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

// Every following service_ptr will be shared_ptr.
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

struct WandService : AbstractService<Wand> {};
struct MagicWandService : SingleService<MagicWand>, Overrides<WandService> {};
struct FireWandService : SingleService<FireWand>, Overrides<MagicWandService> {};
struct LavaWandService : SingleService<LavaWand>, Overrides<FireWandService, MagicWandService> {};
struct TricksterService : Service<Trickster, Dependency<WandService>> {};
struct WizardService : Service<Wizard, Dependency<MagicWandService>> {};
struct FireMageService : Service<FireMage, Dependency<FireWandService>> {};

// This is the init function, we are initiating what we need to make the container behave correctly.
Container makeContainer() {
	// This is the make function, we are initiating what we need to make the main() work.
	Container container;
	
	// MagicWand is the first, because it's the highest non-abstract service in the hierarchy.
	container.instance<MagicWandService>();
		
	// FireWand is the second, because it's the second service in the hierarchy.
	container.instance<FireWandService>();
		
	// LavaWand is the last, because it's the last service in the hierarchy.
	container.instance<LavaWandService>();
	
	return container;
}

int main()
{
	// The container type will be MyContainer.
	auto container = makeContainer();
	
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
	// which grants it priority (see MyContainer::init() for more detail).
	// A misconfigured hierarchy may lead to incorrect result. Invert instance calls and see by yourself.
	wizard.doTrick();
	
	// The trickster will show "It's doing lava tricks!"
	// because LavaWand is overriding FireWand.
	fireMage.doTrick();
	
	return 0;
}
