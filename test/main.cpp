#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <pqxx/pqxx>
#include <util.hpp>
#include <database.hpp>
using namespace std;
namespace d = database;
namespace o = database_online;

#define DATABASE_CONNECTION "dbname=pet user=borges password=JSG3bor_g873sqlptgs78b hostaddr=127.0.0.1 port=5432"
#define DATABASE_CONNECTION_SECURITY "dbname=security user=borges password=JSG3bor_g873sqlptgs78b hostaddr=127.0.0.1 port=5432"
/**
 selects:
  - to retry time in minutes in session:
  SELECT EXTRACT(EPOCH FROM (CURRENT_TIMESTAMP - (select date from head where id = '123')))/60;
 */

//void* CB;
pqxx::connection_base* CB;

int main() {
 try {
	u::error::set_trace();
	d::database_connection = DATABASE_CONNECTION;
	// d::database_connection = DATABASE_CONNECTION_SECURITY;

	o::Table t = { "category", { { "bind_category_product_type", { "category_id", "id" } } } };

	o::init_database_graph();
	// decltype(o::database_graph) db_graph = { { "category", { "bind_category_product_type", { "category_id", "id" } } }};
	// decltype(o::database_graph) db_graph = { { "category", { "category", { { "bind_category_product_type", { "category_id", "id" } } } } }, { "bind_category_product_type", { "bind_category_product_type", { { "category", { "id", "category_id" } }, { "product_type", { "id", "product_type_id" } } } } }, { "users", { "users", { { "person", { "email1", "person_email1" } }, { "person", { "id", "person_id" } } } } }, { "product_type", { "product_type", { { "bind_category_product_type", { "product_type_id", "id" } }, { "product", { "produt_type_id", "id" } } } } }, { "product", { "product", { { "product_type", { "id", "produt_type_id" } }, { "sale", { "id", "sale_id" } } } } }, { "payment_type", { "payment_type", { { "sale", { "payment_type_id", "id" } } } } }, { "person", { "person", { { "users", { "person_email1", "email1" } }, { "users", { "person_id", "id" } }, { "sale", { "client_id", "id" } }, { "sale", { "seller_id", "id" } } } } }, { "sale", { "sale", { { "product", { "sale_id", "id" } }, { "payment_type", { "id", "payment_type_id" } }, { "person", { "id", "client_id" } }, { "person", { "id", "seller_id" } } } } } };
	decltype(o::database_graph) db_graph = database_offline::init_database_graph();

	// for(const auto& [tname, table] : o::database_graph) {
	for(const auto& [tname, table] : db_graph) {
		cout << tname << "\n";
		for(const auto& [foreign_table, col_name] : table.foreign_table) {
			const auto& [fcol, col] = col_name;
			cout << "\t" << foreign_table << "." << fcol << " = " << tname << "." << col << "\n";
		}
	}

	std::cout << "initing checking...\n";
	for(auto [tn, t] : o::database_graph) {
		if(!db_graph.count(tn)) std::cout << "FUCK ERROR TN";
		if(t.name != o::database_graph.at(tn).name) std::cout << "FUCK ERROR T.NAME";

		for(auto [ft, col] : t.for_range_foreign_table()) {
			if(!db_graph.at(tn).foreign_table.count(ft)) std::cout << "FUCK ERROR FT";

			auto [x, y] = col;
			bool c = false;
			for(auto [qt, qcol] : db_graph.at(tn).foreign_table) {
				auto [X, Y] = qcol;
				if(x == X && y == Y && ft == qt) c = true;
			}
			if(!c) std::cout << "FUCK ERROR COL";
		}
	}
	std::cout << "ending checking...\n";
	
	// std::cout << "\n\"" << o::join({"payment_type", "category", "product_type", "sale"}) << "\"\n";
	// std::cout << "\n\"" << o::join({"payment_type", "category", "product_type"}) << "\"\n";
	// std::cout << "\n" << o::join({"category", "product_type"}) << "\"\n";
	// std::cout << "\n" << o::join({"person", "users"}) << "\"\n";
	// std::cout << "\n" << o::join({"person", "category"}) << "\"\n";
	// std::cout << "\n" << o::join({"users", "category"}) << "\"\n";
	// std::cout << "\n" << o::join({"role", "users"}) << "\"\n";
	// std::cout << "\n" << o::join({"role", "users", "system"}) << "\"\n";

	auto person = database_obj_str::map_table<std::unordered_map>("person");
	std::unordered_map<std::string, database_obj_str::Field> persox = database_offline::map_table("person");
	std::string ij = database_offline::sql_inner_join( { "person", "category" } );

	// for(auto& [key, field] : person)
	// 	field.print();
	
	// person.at("name") = "Luiz Alberto Borges Jr.";

	// for(auto& [key, field] : person)
	// 	field.print();
	
	// auto R = database::selectr1("SELECT * FROM person where id=16;");

	// for(auto i : R)
	// 	person.at(u::to_str(i.name())) = i.is_null() ? "" : i.as<std::string>();

	// // for(auto& [key, field] : person)
	// // 	field.print();
	
	// person.at("phone1") = "31975339495";
	// person.at("email2") = "mariaehumsonhoumaeternamagiaumaforcaquenosalerta@ufmg.br";

	// database_obj_str::update(person);
	// database_obj_str::del(person);
	// person.at("name") = "MARIA JOAQUINA";
	// person.at("email1") = "maria@uol.com";
	// person.at("phone1") = "31975339498";
	// person.at("birth") = "24/10/1958";
	// database_obj_str::insert(person);
	
	std::cout << database_obj_str::map_table_str("person") << "\n";

	std::cout << "\n\n====================================================\n\n";
	std::cout << database_online::init_database_graph_str() << "\n";
 } catch (const std::exception &e) {
	err(e.what());
	// cout << u::error::get_trace();
	return 1;
 }
}























