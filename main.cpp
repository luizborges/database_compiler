/**
 * @file main.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-10-27
 * 
 * @copyright Copyright (c) 2021
 * @despriction: Este programa recebe um arquivo, e procura pelas funções definidas na estrutura User.target.
 * Ele então substitui cada uma das funções predefinidas, pelo resultado das chamadas de funções predefinidas, com os argumentos passados pelo usuário.
 * 
 */

////////////////////////////////////////////////////////////////////////////////
// Includes - default libraries - C
////////////////////////////////////////////////////////////////////////////////
#include <getopt.h> // gnu library to get command line options

////////////////////////////////////////////////////////////////////////////////
// Includes - default libraries - C++
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <deque>
#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <fstream>
#include <filesystem>
#include <iostream>

#include <pqxx/pqxx>

////////////////////////////////////////////////////////////////////////////////
// Includes - system dependent libraries
////////////////////////////////////////////////////////////////////////////////
#if defined(unix) || defined(__unix) || defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) // Unix (Linux, *BSD, Mac OS X)
#include <unistd.h>                                                                                    // unix standard library
#include <sys/syscall.h>
#include <sys/types.h>
#include <dirent.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Includes - my libraries
////////////////////////////////////////////////////////////////////////////////
#include <util.hpp>
#include <token.hpp>
#include <database.hpp>
//#include <headers/stackTracer.h>
////////////////////////////////////////////////////////////////////////////////
// Includes - namespace
////////////////////////////////////////////////////////////////////////////////

/*
// void database_online::walk()
{ try {
 } catch (const std::exception &e) { throw err(e.what()); }
}
*/
////////////////////////////////////////////////////////////////////////////////
// class
////////////////////////////////////////////////////////////////////////////////

// class Token_Target {
//     std::string name;
//     std::string block_begin;
//     std::string block_end;
// };

enum class Block_Type {
    comment_line,
    comment_block,
    string,
    database_offline_sql_inner_join,
    database_offline_map_table,
    database_offline_init_database_graph,
    common_code
};

std::string get_block_type_str(const Block_Type block_type);

/**
 * @brief Esta classe representa um bloco do arquivo de entrada.
 * Este bloco quer dizer que tipo de dados está sendo representado ali, os tipos são descritos pela enum class Block_Type.
 * O campo 'std::string output;' somente tem o valor diferente de 'std::string in_content;' para os casos em que o programa substitui a entrada 
 * por outro valor, que são, quando ele reconhece as funções da biblioteca <database_offline.hpp>.
 */
class Block {
 private:
    size_t pos_begin;
    size_t size;
    Block_Type type;
    std::string in_content;
    std::string output;

 public:
    Block(const Block_Type type, const size_t pos_begin, const size_t size)
    { try {
        // this->type_str = type_str;
        this->pos_begin = pos_begin;
        this->size = size;
        this->type = type;
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    Block_Type get_type() const {
        return type;
    }

    size_t get_pos_begin() const {
        return pos_begin;
    }

    size_t get_size() const {
        return size;
    }

    std::string get_input_content() const {
        return in_content;
    }

    void set_input_content_from_file_input(const std::string& original_input)
    { try {
        if(original_input.empty()) throw err("String input file is empty.");

        const auto [exists_tables, content_pos_begin, content_size] = token::find_block(original_input, "(", ")", pos_begin, size);
        if(!exists_tables) {
            throw err("Parameters of functions does not exists_tables - Subblock searched: \'(\' until \')\'. Block type: %s, "
            "file input size: %zu, block content position begin: %zu, block content size: %zu, subblock position begin: %zu, subblock size: %zu",
            get_block_type_str(type), original_input.size(), pos_begin, size, content_pos_begin, content_size);
        }

        in_content = original_input.substr(content_pos_begin, content_size);
        in_content.pop_back(); // remove character ')'
        in_content.erase(0, 1);

     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void set_output(const std::string& output)
    { try {
        if(output.empty())
            throw err("Output block is empty. Input content of block: \"%s(%s)\".", get_block_type_str(type).c_str(), in_content.c_str());
        this->output = output;
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    std::string get_output(const std::string& input) const
    { try {
        if(input.empty())
            throw err("Input string file that generated blocks are empty.");
        
        if(type == Block_Type::database_offline_sql_inner_join
            || type == Block_Type::database_offline_map_table
            || type == Block_Type::database_offline_init_database_graph)
                return output;
        
        else return input.substr(pos_begin, size); // o tipo não é nenhum dos tipos tem uma saída própria, porém é um pedaço do input file
        
     } catch (const std::exception &e) { throw err(e.what()); }
    }
};

class User {
 private:
    bool verbose_mode = false; // modo verbose -> imprime ao final quais são os arquivos de entrada, saída e a conexão com o banco de dados. - não tem efeito para mensagem de erro.

    std::string fin; // file input name
    std::string fout; // file output name
    std::string database_connection; // conexão com o banco de dados para buscar os dados que serão escritos no arquivo de saída
    std::string in; // file input in string format
    std::string out; // output generate
    std::string out_file; // output that is in the original fout

    /** Cada campo significa:
     * Este mapa serve para mapear os blocos que serão substituídos no arquivo de entrada.
     * Serve para identificar o bloco, marcar onde na string que representa o arquivo inicial começa o bloco, e o número de characteres que o bloco contém.
     * @obs: o arquivo de entrada é inserido em uma string.
     * @std::string: string que será buscada no arquivo de entrada, para marcar que o foi encontrado o bloco de substituição do tipo marcado em @arg(Block_Type)
     * @size_t: é a posição na string que representa o arquivo inicial, onde começa o bloco.
     * @size_t: é o tamanho, número de characteres, que o bloco contém.
     * @Block_Type: é o tipo do bloco, ou seja, é um marcador para o programa saber qual é o tipo do bloco. Se é um comentário de linha, uma string, 
     * um comentário de bloco, ou se é uma função de substituição e qual é a função.
     */
    std::unordered_map<std::string, std::tuple<size_t, size_t, Block_Type>> target = { 
        { "database_offline::sql_inner_join(", { std::string::npos, 0, Block_Type::database_offline_sql_inner_join } }, 
        { "database_offline::map_table(", { std::string::npos, 0, Block_Type::database_offline_map_table } },
        { "database_offline::init_database_graph(", { std::string::npos, 0, Block_Type::database_offline_init_database_graph } } };
    
    std::vector<Block> in_block; // file input in block format
 public:

    void set_verbose_mode(const bool verbose_mode)
    { try {
        this->verbose_mode = verbose_mode;
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    bool is_verbose_mode() const {
        return verbose_mode;
    }

    std::string get_file_input_name() const {
        return fin;
    }

    std::string get_file_output_name() const {
        return fout;
    }

    std::string get_database_connection() const {
        return database_connection;
    }

    void init_database() {
        if(this->database_connection.empty()) return;

        database::database_connection = database_connection;
        database_online::init_database_graph(); // necessário para executar funções como database_online::join()
    }

    void set_file_input(const std::string& fin)
    { try {
        if(fin.empty()) throw err("input file name is an empty string");
        this->fin = fin;
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void set_file_output(const std::string& fout)
    { try {
        if(fout.empty()) throw err("output file name is an empty string");
        this->fout = fout;
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void set_database_connection(const std::string& database_connection)
    { try {
        if(database_connection.empty()) throw err("database connection is an empty string");
        this->database_connection = database_connection;
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void check_input_arg() const
    { try {
        if(fin.empty()) throw err("File input name not passed as programming argument. See documentation.");
        if(!std::filesystem::exists(fin)) throw err("Input file name is not an real archive. input file name: \"%s\"", fin.c_str());
        if(fout.empty()) throw err("File output name not passed as programming argument. See documentation.");
        // if(database_connection.empty()) throw err("Database connection not passed as programming argument. See documentation.");
        // std::cout << "fin: " << fin << "\nfout: " << fout << "\ndb: " << database_connection << "\n";
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void open_file_input()
    { try {
        std::ifstream t(fin);
        std::stringstream buffer;
        buffer << t.rdbuf();
        in = buffer.str();
        // std::cout << "in: \"" << in << "\"\n";
        // const auto [b, i, e] = token::find_str(in);
        // std::cout << "b: " << u::to_str(b) << ", i: " << i << ", c: " << e << "\n";
        // const auto [b2, i2, e2] = token::find_block(in, "test", "test");
        // std::cout << "b2: " << u::to_str(b2) << ", i2: " << i2 << ", c2: " << e2 << "\n";
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void break_file_input_in_blocks()
    { try {
        size_t pos = 0;
        while(pos < in.size())
        {
            ////////////////////////////////////////////////////////////////////////////////
            // procura qual dos tipos é o primeiro que aparece na string input
            ////////////////////////////////////////////////////////////////////////////////
            const auto [is_comment_line, comment_line_begin, comment_line_size] = token::find_block(in, "//", "\n", pos);
            const auto [is_comment_block, comment_block_begin, comment_block_size] = token::find_block(in, "/*", "*/", pos);
            const auto [is_str, str_begin, str_size] = token::find_str(in, pos);
            for(auto& [token_target, token_target_info] : this->target) {
                auto& [token_target_begin, token_target_size, token_target_type] = token_target_info;
                const auto [is_token_target, _token_target_begin_, _token_target_size_] = token::find_block(in, token_target, ")", pos);
                token_target_begin = _token_target_begin_;
                token_target_size = _token_target_size_;
            }

            // std::cout << "in:\n======================\n" << in << "======================\n";
            // std::cout << "pos: " << pos << ", cl: " << comment_line_begin << ", cb: " << comment_block_begin << ", str: " << str_begin << "\n";

            ////////////////////////////////////////////////////////////////////////////////
            // escolhe o menor para continuar a busca
            // cria um block e armazena ele 
            ////////////////////////////////////////////////////////////////////////////////
            auto pos_first = comment_line_begin;
            auto block_size = comment_line_size;
            auto block_type = Block_Type::comment_line;

            if(pos_first > comment_block_begin) {
                pos_first = comment_block_begin;
                block_size = comment_block_size;
                block_type = Block_Type::comment_block;
            }

            if(pos_first > str_begin) {
                pos_first = str_begin;
                block_size = str_size;
                block_type = Block_Type::string;
            }

            for(auto& [token_target, token_target_info] : this->target) {
                auto& [token_target_begin, token_target_size, token_target_type] = token_target_info;
                if(pos_first > token_target_begin) {
                    pos_first = token_target_begin;
                    block_size = token_target_size;
                    block_type = token_target_type;
                }
            }

            ////////////////////////////////////////////////////////////////////////////////
            // armazena os blocos
            ////////////////////////////////////////////////////////////////////////////////
            if(pos != pos_first) {
                Block text_block { Block_Type::common_code , pos, pos_first - pos };
                this->in_block.push_back(text_block);
            }

            // std::cout << "pos: " << pos << ", pos_first: " << pos_first << ", block_size: " << block_size << ", block_type: " << get_block_type_str(block_type) << "\n";
            if(pos_first == std::string::npos || block_size == std::string::npos) break; // fim da busca
            pos = pos_first + block_size; // atualiza a nova posição no arquivo de entrada para realizar a busca

            Block new_block { block_type, pos_first, block_size };
            this->in_block.push_back(new_block);
        }
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void process_target_block()
    { try {
        for(auto& block : in_block) {
            if(block.get_type() == Block_Type::database_offline_sql_inner_join)
                run_sql_inner_join(block);
            else if(block.get_type() == Block_Type::database_offline_map_table)
                run_map_table(block);
            else if(block.get_type() == Block_Type::database_offline_init_database_graph)
                run_init_database_graph(block);
        }
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void print_blocks()
    { try {
        size_t pos_next = 0;
        bool init = false;
        for(auto& block : in_block) {
            std::cout << "block type: " << get_block_type_str(block.get_type()) << ", block pos_begin: " << block.get_pos_begin() << ", block size: " << block.get_size() << ", block pos_end+1: " << block.get_pos_begin() + block.get_size() << "\n";
            if(init &&  pos_next != block.get_pos_begin()) {
                throw err("fuck you");
            } else std::cout << "check ok...\n";
            pos_next =  block.get_pos_begin() + block.get_size();
            init = true;

        }
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void generate_output()
    { try {
        for(auto& block : in_block)
            out += block.get_output(in);
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void open_file_output()
    { try {
        bool exist = std::filesystem::exists(fout);
        if(exist) {
            std::ifstream t(fout);
            std::stringstream buffer;
            buffer << t.rdbuf();
            out_file = buffer.str();
        }
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    bool is_file_output_and_generate_output_are_equal() const
    { try {
        // std::cout << "o: \""<< out << "\"\nx: \"" << out_file << "\"\n";
        if(out == out_file) return true;
        else return false;
     } catch (const std::exception &e) { throw err(e.what()); }
    }

    void make_generate_output_to_be_file_output() const
    { try {
        std::ofstream output_file (fout);
        if(!output_file.is_open()) throw err("Could not open output file.");
        output_file << out;
        if(!output_file.good()) throw err("Something going wrong in writing output file. Writting in output file is not std::ios::good().");
        output_file.close();
     } catch (const std::exception &e) { throw err("%s\nOutput file name is: \"%s\"", e.what(), fout.c_str()); }
    }

 private:
    void run_sql_inner_join(Block& block)
    { try {
        block.set_input_content_from_file_input(in); // recebe o conteúdo que está entre a chamada de função da database_offline::xxx()
        ////////////////////////////////////////////////////////////////////////////////
        // formato aceitável: ( { "table1", "table2", ..., "tablen" }, "database_connection" )
        // consegue as strings que representam as tabelas -> { "table1", "table2", ..., "tablen" }
        // Os characteres '{' e '}' são obrigatórios
        // consegue a string que representa a conexão com o banco de dados -> "database_connection"
        //  o database_connection é opcional
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        // descobre as tabelas alvos do inner join
        ////////////////////////////////////////////////////////////////////////////////
        const std::string content = block.get_input_content();
        const auto& [exists_tables, tables_pos_begin, tables_size] = token::find_block(content, "{", "}");
        if(!exists_tables)
            throw err("database_offline::sql_inner_join is malformed. Expected \'{\' and \'}\' in content: \"database_offline::sql_inner_join(%s)\"", content.c_str());

        std::string tables_str = content.substr(tables_pos_begin, tables_size);
        tables_str.erase(0, 1); // retira o character '{'
        tables_str.pop_back(); // retira o character '}'

        std::unordered_set<std::string> target_tables_inner_join;
        size_t pos_begin_search = 0;
        for(bool searching_tables = true; searching_tables; ) {
            const auto& [exists_table, table_pos_begin, table_size] = token::find_str(tables_str, pos_begin_search);
            if(exists_table) {
                std::string table = tables_str.substr(table_pos_begin, table_size);
                table.erase(0, 1); // remove '"' from begining
                table.pop_back(); // remove '"' from end
                target_tables_inner_join.emplace(table);
                pos_begin_search = table_pos_begin + table_size; // atualiza o valor da busca para buscar a próxima tabela
                if(pos_begin_search >= tables_str.size()) searching_tables = false;
            } else {
                searching_tables = false;
            }
        }

        if(target_tables_inner_join.size() < 2)
            throw err("database_offline::sql_inner_join is malformed. Expected at least 2 tables in content, found %zu tables. \"database_offline::sql_inner_join(%s)\"", target_tables_inner_join.size(), content.c_str());

        ////////////////////////////////////////////////////////////////////////////////
        // retira a conexão com o banco de dados, se houver
        ////////////////////////////////////////////////////////////////////////////////
        std::string db_connection;
        size_t begin_db_connection_search =  tables_pos_begin + tables_size;
        if(begin_db_connection_search < content.size()) {
            const auto& [exists_db_connection, db_connection_pos_begin, db_connection_size] = token::find_str(content, begin_db_connection_search);
            if(exists_db_connection) {
                db_connection = content.substr(db_connection_pos_begin, db_connection_size);
                db_connection.erase(0, 1); // remove character '"' from begining
                db_connection.pop_back(); // remove character '"' from end
            }
        }
        
        ////////////////////////////////////////////////////////////////////////////////
        // executa a função alvo e guarda o resultado
        ////////////////////////////////////////////////////////////////////////////////
        std::string output;
        if(db_connection.empty()) // não foi passado a conexão do banco de dados.
            output = database_online::join(target_tables_inner_join);
        else { // foi passado o database.
            std::string database_connection = database::database_connection; // guarda o database atual
            database::database_connection = db_connection; // insere o novo database
            database_online::init_database_graph(); // inicializa o grafo que representa o banco de dados
            output = database_online::join(target_tables_inner_join); // realiza o inner join
            database::database_connection = database_connection; // volta a conexão anterior
            if(!database_connection.empty()) database_online::init_database_graph(); // gera o grafo do banco de dados
        }

        block.set_output("\"" + output + "\""); // guarda o resultado no bloco
        // std::cout << "output :\"" << output << "\"\n";
     } catch (const std::exception &e) { throw err("%sInput content of block: \"%s(%s)\".", e.what(), get_block_type_str(block.get_type()).c_str(), block.get_input_content().c_str()); }
    }

    void run_map_table(Block& block)
    { try {
        block.set_input_content_from_file_input(in); // recebe o conteúdo que está entre a chamada de função da database_offline::xxx()
        ////////////////////////////////////////////////////////////////////////////////
        // formato aceitável: ( "table1", "database_connection" )
        // "table1" -> é obrigatório.
        // consegue a string que representa a conexão com o banco de dados -> "database_connection"
        //  o database_connection é opcional
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        // descobre a tabela da função database_obj_str::map_table()
        ///////////////////////////////////////////////////////////////////////////////
        const std::string content = block.get_input_content();
        const auto& [exists_table, table_pos_begin, table_size]= token::find_str(content);
        if(!exists_table)
            throw err("database_offline::map_table is malformed. Expected string that reprents table database in content: \"database_offline::map_table(%s)\"", content.c_str());
        
        std::string table = content.substr(table_pos_begin, table_size);
        table.erase(0, 1); // remove '"' from beginning
        table.pop_back(); // remove '"' from end

        ////////////////////////////////////////////////////////////////////////////////
        // retira a conexão com o banco de dados, se houver
        ////////////////////////////////////////////////////////////////////////////////
        std::string db_connection;
        size_t begin_db_connection_search =  table_pos_begin + table_size;
        if(begin_db_connection_search < content.size()) {
            const auto& [exists_db_connection, db_connection_pos_begin, db_connection_size] = token::find_str(content, begin_db_connection_search);
            if(exists_db_connection) {
                db_connection = content.substr(db_connection_pos_begin, db_connection_size);
                db_connection.erase(0, 1); // remove character '"' from begining
                db_connection.pop_back(); // remove character '"' from end
            }
        }
        
        ////////////////////////////////////////////////////////////////////////////////
        // executa a função alvo e guarda o resultado
        ////////////////////////////////////////////////////////////////////////////////
        std::string output;
        if(db_connection.empty()) // não foi passado a conexão do banco de dados.
            output = database_obj_str::map_table_str(table);
        else { // foi passado o database.
            std::string database_connection = database::database_connection; // guarda o database atual
            database::database_connection = db_connection; // insere o novo database
            output = database_obj_str::map_table_str(table, db_connection);
            database::database_connection = database_connection;
            if(!database_connection.empty()) database_online::init_database_graph(); // gera o grafo do banco de dados
        }

        block.set_output(output); // guarda o resultado no bloco
        // std::cout << "output :\"" << output << "\"\n";
     } catch (const std::exception &e) { throw err("%sInput content of block: \"%s(%s)\".", e.what(), get_block_type_str(block.get_type()).c_str(), block.get_input_content().c_str()); }
    }

    void run_init_database_graph(Block& block)
    { try {
        block.set_input_content_from_file_input(in); // recebe o conteúdo que está entre a chamada de função da database_offline::xxx()
        ////////////////////////////////////////////////////////////////////////////////
        // formato aceitável: ( "database_connection" )
        // consegue a string que representa a conexão com o banco de dados -> "database_connection"
        //  o database_connection é opcional
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        // retira a conexão com o banco de dados, se houver
        ///////////////////////////////////////////////////////////////////////////////
       const std::string content = block.get_input_content();
        std::string db_connection;
        const auto& [exists_db_connection, db_connection_pos_begin, db_connection_size] = token::find_str(content);
        if(exists_db_connection) {
            db_connection = content.substr(db_connection_pos_begin, db_connection_size);
            db_connection.erase(0, 1); // remove character '"' from begining
            db_connection.pop_back(); // remove character '"' from end
        }
        
        ////////////////////////////////////////////////////////////////////////////////
        // executa a função alvo e guarda o resultado
        ////////////////////////////////////////////////////////////////////////////////
        std::string output;
        if(db_connection.empty()) // não foi passado a conexão do banco de dados.
            output = database_online::init_database_graph_str();
        else { // foi passado o database.
            std::string database_connection = database::database_connection; // guarda o database atual
            database::database_connection = db_connection; // insere o novo database
            output = database_online::init_database_graph_str();
            database::database_connection = database_connection;
            if(!database_connection.empty()) database_online::init_database_graph(); // gera o grafo do banco de dados
        }

        block.set_output(output); // guarda o resultado no bloco
        // std::cout << "dboutput :\"" << output << "\"\n";
     } catch (const std::exception &e) { throw err("%sInput content of block: \"%s(%s)\".", e.what(), get_block_type_str(block.get_type()).c_str(), block.get_input_content().c_str()); }
    }


    // int get_file_input_line(size_t pos) const
    //  { try {
        
    //  } catch (const std::exception &e) { throw err(e.what()); }
    // }

};


////////////////////////////////////////////////////////////////////////////////
// functions - head
////////////////////////////////////////////////////////////////////////////////
User get_program_argument(int argc, char **argv);

std::string get_block_type_str(const Block_Type block_type);

////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) 
{
    auto u = get_program_argument(argc, argv);
    try {
        u.open_file_input();
        u.init_database();
        u.break_file_input_in_blocks();
        // u.print_blocks();
        u.process_target_block();
        u.generate_output();
        u.open_file_output();
        if(!u.is_file_output_and_generate_output_are_equal()) {
            if(u.is_verbose_mode()) std::cerr << "Input file and output file are different. New output file wrote.\nInput file: \'" << u.get_file_input_name() << "\'\nOutput file: \'" << u.get_file_output_name() << "\'.\nDatabase connection: \'" << u.get_database_connection() << "\'.\n";
            u.make_generate_output_to_be_file_output();
        } else
            if(u.is_verbose_mode()) std::cerr << "Input file and output file are equal.\nInput file: \'" << u.get_file_input_name() << "\'\nOutput file: \'" << u.get_file_output_name() << "\'.\nDatabase connection: \'" << u.get_database_connection() << "\'.\n";
    } catch (const std::exception &e) { 
        err("%sFile input name: '%s'.\nFile output name: '%s'.\nDatabase connection: '%s'.", e.what(), u.get_file_input_name().c_str(), u.get_file_output_name().c_str(), u.get_database_connection().c_str());
        return EXIT_FAILURE;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// functions
////////////////////////////////////////////////////////////////////////////////
User 
get_program_argument(int argc, char **argv)
{ try {
    User user;
    static struct option long_options[] =
    {
        {"input", required_argument, NULL, 'i'},
        {"output", required_argument, NULL, 'o'},
        {"database_connection", required_argument, NULL, 'd'},
        {"verbose", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

    int opt;
    while ((opt = getopt_long_only(argc, argv, "", long_options, NULL)) != -1)
    {
        // check to see if a single character or long option came through
        switch (opt)
        {
            case 'i':
                user.set_file_input(u::to_str(optarg)); // or copy it if you want to
                break;
            case 'o':
                user.set_file_output(u::to_str(optarg)); // or copy it if you want to
                break;
            case 'd':
                user.set_database_connection(u::to_str(optarg));; // or copy it if you want to
                break;
            case 'v':
                user.set_verbose_mode(true);
                break;
            default:
                throw err("Programming option incorrect: \'%c\'. See documentation.", opt);
        }
    }
    
    user.check_input_arg();
    return user;
 } catch (const std::exception &e) { throw err(e.what()); }
}

std::string get_block_type_str(const Block_Type block_type)
{ try {
    if(block_type == Block_Type::comment_line) return "comment_line";
    if(block_type == Block_Type::comment_block) return "comment_block";
    if(block_type == Block_Type::string) return "string";
    if(block_type == Block_Type::database_offline_sql_inner_join) return "database_offline_sql_inner_join";
    if(block_type == Block_Type::database_offline_map_table) return "database_offline_map_table";
    if(block_type == Block_Type::database_offline_init_database_graph) return "database_offline_init_database_graph";
    if(block_type == Block_Type::common_code) return "common_code";
    
    throw err("Block Type is undefined. Block Type: %d", static_cast<int>(block_type));
    return "";
 } catch (const std::exception &e) { throw err(e.what()); }
}

/*
// void database_online::walk()
{ try {
 } catch (const std::exception &e) { throw err(e.what()); }
}
*/
