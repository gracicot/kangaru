#include <kangaru-prev/kangaru.hpp>
#include <iostream>

// This macro is used as a shortcut to use kgr::Method. Won't be needed in C++17
#define METHOD(...) ::kgr::method<decltype(__VA_ARGS__), __VA_ARGS__>

// The following classes are user classes.
// As you can see, this library is not intrusive and don't require modifications
struct Credential {};

struct Connection {
	// The connect needs some credential
	void connect(Credential const&) {
		std::cout << "connection established" << std::endl;
	}
};

struct Database {
	// A database needs a connection
	explicit Database(Connection const&) {
		std::cout << "database created" << std::endl;
	}

	// For the sake of having a method to call
	void commit() {
		std::cout << "database commited" << std::endl;
	}
};


// Service definitions.
// We define all dependencies between classes,
// and tell the container how to map function parameter to those definitions.

// Simple injectable service by value
struct CredentialService : kgr::service<Credential> {};

// Connection service is single,
// and need the connect function to be called on creation
struct ConnectionService : kgr::single_service<Connection>,
	kgr::autocall<METHOD(&Connection::connect)> {};


// Database is also a single, and has a connection as dependency
struct DatabaseService : kgr::single_service<Database, kgr::dependency<ConnectionService>> {};

// The service map maps a function parameter type to a service definition
// We also want to map a Database argument type for the example.
auto service_map(Database const&) -> DatabaseService;
auto service_map(Credential const&) -> CredentialService;

int main() {
	kgr::container container;

	// Get the database.
	// The database has a connection injected,
	// and the connection had the connect function called before injection.
	auto&& database = container.service<DatabaseService>();

	// Commit the database
	database.commit();

	// Let `function` be a callable object that takes mapped services.
	auto function = [](Credential c, Database& db) {
		// Do stuff with credential and database
	};

	// The function is called with it's parameter injected automatically.
	container.invoke(function);

	// The programs outputs:
	//   connection established
	//   database created
	//   database commited
}
