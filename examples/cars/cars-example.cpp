#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "kangaru.hpp"

struct CarsContainer;

struct Fuel {
	auto getPrice() const noexcept -> double {
		return price_;
	}

	auto setPrice(double price) -> void {
		if (price < 0.0) 
			throw std::runtime_error{"Price cannot be negative"};
		price_ = price;
	}
protected:
	Fuel() = default;
private:
	double price_;
};

struct Car {
	auto refuel(size_t litres) const noexcept -> double {
		return litres * fuel_->getPrice();
	}

	auto setFuel(std::shared_ptr<Fuel> fuel) -> void {
		fuel_ = std::move(fuel);
	}
protected:
	explicit Car(std::shared_ptr<Fuel> fuel) : fuel_{std::move(fuel)} {}
private:
	std::shared_ptr<Fuel> fuel_;
};

struct Petrol : Fuel {
	Petrol() = default;
};

struct PremiumPetrol : Petrol {
	PremiumPetrol() = default;
};

struct Diesel : Fuel {
	Diesel() = default;
};

struct OpelAstra : Car {
	explicit OpelAstra(std::shared_ptr<Fuel> fuel) :
		Car{std::move(fuel)} {}
};

struct NissanQuashqai : Car {
	explicit NissanQuashqai(std::shared_ptr<Fuel> fuel) :
		Car{std::move(fuel)} {}
};

struct HondaHRV : Car {
	explicit HondaHRV(std::shared_ptr<Fuel> fuel) :
		Car{std::move(fuel)} {}
};

struct HondaHRVDiesel : HondaHRV {
	explicit HondaHRVDiesel(std::shared_ptr<Fuel> fuel) :
		HondaHRV{std::move(fuel)} {}
};

struct CarsContainer : kgr::Container {
	CarsContainer() = default;
protected:
	virtual auto init() -> void override final;
};

auto CarsContainer::init() -> void {
	service<Petrol>()->setPrice(130.70);
	service<PremiumPetrol>()->setPrice(144.50);
	service<Diesel>()->setPrice(135.30);
}

namespace kgr {

template<> struct Service<Fuel>          : NoDependencies, Single {};
template<> struct Service<Petrol>        : NoDependencies, Overrides<Fuel> {};
template<> struct Service<PremiumPetrol> : NoDependencies, Single {};
template<> struct Service<Diesel>        : NoDependencies, Single {};

template<> struct Service<OpelAstra>      : Dependency<Petrol> {};
template<> struct Service<NissanQuashqai> : Dependency<Petrol> {};
template<> struct Service<HondaHRV>       : Dependency<PremiumPetrol> {};
template<> struct Service<HondaHRVDiesel> : Dependency<Diesel> {};

}  // namespace kgr



int main() {
	auto garage = kgr::make_container<CarsContainer>();

	auto astra1     = garage->service<OpelAstra>();
	auto astra2     = garage->service<OpelAstra>();
	auto quashqai   = garage->service<NissanQuashqai>();
	auto hrv        = garage->service<HondaHRV>();
	auto hrv_diesel = garage->service<HondaHRVDiesel>();
	auto premium    = garage->service<PremiumPetrol>();

	astra2->setFuel(premium);

	std::cout << std::boolalpha << std::fixed << std::setprecision(2);
	std::cout << "`astra1` and `astra2` is the same auto:  " <<
		(astra1 == astra2) <<
		"\n`hrv` and `hrv_diesel` is the same auto: " <<
		(hrv == hrv_diesel) <<
		"\npremium petrol is single:                " << 
		(premium == garage->service<PremiumPetrol>()) <<
		"\nPremium petrol costs " << premium->getPrice() <<
		" per litre"
		"\n\nRefuel costs:"
		"\n\tOpel Astra(1),   50 litres: " << astra1->refuel(50) <<
		"\n\tOpel Astra(2),   50 litres: " << astra2->refuel(50) <<
		"\n\tNissan Quashqai, 30 litres: " << quashqai->refuel(30) <<
		"\n\tHonda"
		"\n\t  HR-V,          30 litres: " << hrv->refuel(30) <<
		"\n\t  HR-V Diesel,   30 litres: " << hrv_diesel->refuel(30) <<
		'\n';
	premium->setPrice(147.20);
	std::cout << "Update premium petrol price to " << premium->getPrice() <<
		"\nNow refuel of Honda HR-V (30 litres) costs " <<
		hrv->refuel(30) << 
		'\n';
}

