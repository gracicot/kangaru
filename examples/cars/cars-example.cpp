#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <kangaru/kangaru.hpp>

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

	auto setFuel(Fuel* fuel) -> void {
		fuel_ = std::move(fuel);
	}
protected:
	explicit Car(Fuel* fuel) : fuel_{std::move(fuel)} {}
private:
	Fuel* fuel_;
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
	explicit OpelAstra(Fuel& fuel) :
		Car{&fuel} {}
};

struct NissanQuashqai : Car {
	explicit NissanQuashqai(Fuel& fuel) :
		Car{&fuel} {}
};

struct HondaHRV : Car {
	explicit HondaHRV(Fuel& fuel) :
		Car{&fuel} {}
};

struct HondaHRVDiesel : HondaHRV {
	explicit HondaHRVDiesel(Fuel& fuel) :
		HondaHRV{fuel} {}
};

struct FuelService           : kgr::abstract_service<Fuel> {};
struct PetrolService         : kgr::single_service<Petrol>, kgr::overrides<FuelService> {};
struct PremiumPetrolService  : kgr::single_service<PremiumPetrol> {};
struct DieselService         : kgr::single_service<Diesel> {};

struct OpelAstraService      : kgr::service<OpelAstra, kgr::dependency<PetrolService>> {};
struct NissanQuashqaiService : kgr::service<NissanQuashqai, kgr::dependency<PetrolService>> {};
struct HondaHRVService       : kgr::service<HondaHRV, kgr::dependency<PremiumPetrolService>> {};
struct HondaHRVDieselService : kgr::service<HondaHRVDiesel, kgr::dependency<DieselService>> {};

kgr::container makeCarsContainer() {
	kgr::container container;
	
	container.service<PetrolService>().setPrice(130.70);
	container.service<PremiumPetrolService>().setPrice(144.50);
	container.service<DieselService>().setPrice(135.30);
	
	return container;
}

int main() {
	kgr::container garage = makeCarsContainer();

	auto astra1     = garage.service<OpelAstraService>();
	auto astra2     = garage.service<OpelAstraService>();
	auto quashqai   = garage.service<NissanQuashqaiService>();
	auto hrv        = garage.service<HondaHRVService>();
	auto hrv_diesel = garage.service<HondaHRVDieselService>();
	auto& premium   = garage.service<PremiumPetrolService>();

	astra2.setFuel(&premium);

	std::cout << std::boolalpha << std::fixed << std::setprecision(2);
	std::cout << "`astra1` and `astra2` is the same auto:  "
		<< (&astra1 == &astra2)
		<< "\n`hrv` and `hrv_diesel` is the same auto: "
		<< (&hrv == &hrv_diesel)
		<< "\npremium petrol is single:                "
		<< (&premium == &garage.service<PremiumPetrolService>())
		<< "\nPremium petrol costs " << premium.getPrice()
		<< " per litre"
		<< "\n\nRefuel costs:"
		<< "\n\tOpel Astra(1),   50 litres: " << astra1.refuel(50)
		<< "\n\tOpel Astra(2),   50 litres: " << astra2.refuel(50)
		<< "\n\tNissan Quashqai, 30 litres: " << quashqai.refuel(30)
		<< "\n\tHonda"
		<< "\n\t  HR-V,          30 litres: " << hrv.refuel(30)
		<< "\n\t  HR-V Diesel,   30 litres: " << hrv_diesel.refuel(30)
		<< '\n';
		
	premium.setPrice(147.20);
	
	std::cout << "Update premium petrol price to " << premium.getPrice()
		<< "\nNow refuel of Honda HR-V (30 litres) costs "
		<< hrv.refuel(30)
		<< '\n';
}

