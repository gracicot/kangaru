//#include <kangaru-prev/kangaru.hpp>
#include <kangaru/kangaru.hpp>
#include <kangaru/short.hpp>
#include <array>
#include <vector>
#include <fmt/core.h>
#include <cassert>
//#include <iostream>
//#include <string>

/**
 * This example refect snippets of code found in the documentation section 1: Services
 * It explains how to branch containers and operate between them.
 */

//using namespace std::literals;

// Camera is a user class.
//struct Camera {
//	int position;
//};

// Scene too. User class.
//struct Scene {
//	Scene(Camera c, int w = 800, int h = 600) :
//		camera{c.position}, width{w}, height{h} {}
//	
//private:
//	Camera camera;
//	int width;
//	int height;
//};

//struct Screen {
//	Scene& scene;
//	Camera camera;
//};

// This is our service definitions
//struct CameraService : kgr::service<Camera> {};

// SceneService is a single service of Scene, that depends on a camera
//struct SceneService : kgr::single_service<Scene, kgr::dependency<CameraService>> {};

// ScreenService is a single service of Screen, that depends on a scene and camera
//struct ScreenService : kgr::service<Screen, kgr::dependency<SceneService, CameraService>> {};

auto main() -> int {
	struct Camera {
		int id;
	};

	struct Model {
		int id;
	};

	struct Scene {
		explicit constexpr Scene(Camera& c, Model m) : camera{c}, model{m} {}
		Camera& camera;
		Model model;
	};

	struct Movie {
		explicit constexpr Movie(Scene s) : scene{s} {}
		Scene scene;
	};

	auto camera = Camera{.id = 2};
	auto model = Model{.id = 8};
	
	auto camera_source = kangaru::reference_source{camera};
	auto model_source = kangaru::rvalue_source{std::move(model)};
	auto source = kangaru::recursive_source{kangaru::tie(model_source, camera_source)};

	auto injector = kangaru::simple_injector{source};

	injector([&camera](Movie movie) -> void {
 		fmt::print("camera id: {}\nmodel id: {}\nis equal: {}\n",
			movie.scene.camera.id,
			movie.scene.model.id,
			&camera == &movie.scene.camera
		);
		assert(&camera == &movie.scene.camera);
	});

	//auto make = kangaru::constructor<Scene>();
	//auto lambda = [make](auto deduce1, auto... deduce) -> decltype(make(kangaru::exclude_deduction<Scene>(deduce1), kangaru::exclude_deduction<Scene>(deduce)...)) { return make(kangaru::exclude_deduction<Scene>(deduce1), kangaru::exclude_deduction<Scene>(deduce)...); };
	// kangaru::spread_injector test{kgr::ref(rec)};
	//static_assert(kangaru::detail::concepts::callable<decltype(lambda), kangaru::deducer<kangaru::detail::injector::match_any>, kangaru::deducer<kangaru::detail::injector::match_any>>);
	//using G = kangaru::detail::injector::parameter_sequence_t<decltype(lambda), 12>;
	//using G2 = kangaru::detail::injector::injectable_sequence<decltype(lambda), decltype(rec)&, G>::type;
	//using aa = G::patate;
	//using a = G2::patate;
	// lambda(kangaru::deducer<decltype(rec)&>{rec}, kangaru::deducer<decltype(rec)&>{rec});
	//test(lambda);
	//injector([](Scene) {});

	// (void) scene;

	// kgr::container container;
	
	// We create two cameras.
	// Camera camera = container.service<CameraService>();
	// Camera furtherCamera = container.service<CameraService>(14);
	
	// prints 0
	// std::cout << "Camera Position: " << camera.position << '\n';
	
	// prints 14
	// std::cout << "Further Camera Position: " << furtherCamera.position << '\n';
	
	// A Screen has a Scene and a Camera injected in it.
	// Screen screen1 = container.service<ScreenService>();
	// Screen screen2 = container.service<ScreenService>();
	
	// Spoiler: yes they are the same
	// std::cout << "Is both scene the same? " << (&screen1.scene == &screen2.scene ? "yes" : "no") << '\n';
}
