#include <iostream>
#include <string>
std::unordered_map<std::string, database_obj_str::Field> persox = { {"name", {"person", "name", "character varying", true}}, {"phone2", {"person", "phone2", "character varying", false}}, {"phone1", {"person", "phone1", "character varying", true}}, {"email2", {"person", "email2", "character varying", false}}, {"email1", {"person", "email1", "character varying", true, database_obj_str::Constraint::unique}}, {"email3", {"person", "email3", "character varying", false}}, {"birth", {"person", "birth", "date", true}}, {"phone3", {"person", "phone3", "character varying", false}}, {"id", {"person", "id", "integer", true, database_obj_str::Constraint::primary_key}};

std::string sql = "from xxx" + "system JOIN role ON system.id=role.system_id JOIN users_set_head ON system.id=users_set_head.system_id JOIN users_set ON users_set_head.id=users_set.set_id JOIN users ON users_set.user_id=users.id" + "fuck";

{ { "role_set_head", { "role_set_head", { { "system", { "id", "system_id" } }, { "binding_users_role", { "role_set_id", "id" } }, { "role_set", { "set_id", "id" } } } } }, { "binding_users_role", { "binding_users_role", { { "role_set_head", { "id", "role_set_id" } }, { "users_set_head", { "id", "users_set_id" } } } } }, { "role_set", { "role_set", { { "role", { "id", "role_id" } }, { "role_set_head", { "id", "set_id" } } } } }, { "users", { "users", { { "users_set", { "user_id", "id" } } } } }, { "users_set", { "users_set", { { "users_set_head", { "id", "set_id" } }, { "users", { "id", "user_id" } } } } }, { "role", { "role", { { "role_set", { "role_id", "id" } }, { "system", { "id", "system_id" } } } } }, { "system", { "system", { { "role_set_head", { "system_id", "id" } }, { "role", { "system_id", "id" } }, { "users_set_head", { "system_id", "id" } } } } }, { "users_set_head", { "users_set_head", { { "binding_users_role", { "users_set_id", "id" } }, { "users_set", { "set_id", "id" } }, { "system", { "id", "system_id" } } } } } }

#define DATABASE_CONNECTION "dbname=pet user=borges password=JSG3bor_g873sqlptgs78b hostaddr=127.0.0.1 port=5432"
#define DATABASE_CONNECTION_SECURITY  "dbname=security user=borges password=JSG3bor_g873sqlptgs78b hostaddr=127.0.0.1 port=5432"
/**
 selects:
  - to retry time in minutes in session:
  SELECT EXTRACT(EPOCH FROM (CURRENT_TIMESTAMP - (select date from head where id = '123')))/60;
 */
