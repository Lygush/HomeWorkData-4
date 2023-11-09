#include <iostream>
#include <pqxx/pqxx>
#include <vector>

class Clients {
    std::string host{"127.0.0.1"};
    std::string port{"5432"};
    std::string dbname{"HomeWorkData-4-1"};
    std::string user{"postgres"};
    std::string password{"ivan"};
    std::vector<pqxx::connection> connection;
public:
    Clients() {
        try { 
        pqxx::connection con(this->connection_string());
        connection.push_back(std::move(con));
        }
        catch (std::exception& ex) {
            std::cout << ex.what() << std::endl;
        }
    }

    Clients(std::string host, std::string port, std::string dbname, std::string user, std::string password) {
        this->host = host;
        this->port = port;
        this->dbname = dbname; 
        this->user = user; 
        this->password = password; 
        try { 
        pqxx::connection con(this->connection_string());
        connection.push_back(std::move(con));
        }
        catch (std::exception& ex) {
            std::cout << ex.what() << std::endl;
        }
    }

private:
    std::string connection_string() {
        std::string connection {"host=" + this->host + " port=" + this->port +
        " dbname=" + this->dbname + " user=" + this->user + " password=" + this->password};
        return connection;
    }

    void work(std::string& request) {
        try {
            pqxx::work iw(this->get_connection());
            iw.exec(request);
            iw.commit();
        }
        catch(std::exception& ex) {
            std::cout << ex.what();
        }
    }

    auto esc(std::string& value) {
        pqxx::work w(this->get_connection());
        return w.esc(value);
    }

public:
    pqxx::connection& get_connection() {
        return connection.at(0);
    }

    void create_db_struct() {
        try {
            pqxx::transaction t(this->get_connection());
            t.exec( "CREATE TABLE IF NOT EXISTS Client_info ("
                    "id SERIAL PRIMARY KEY,"
                    "name VARCHAR(30) NOT NULL,"
                    "last_name VARCHAR(30),"
                    "email VARCHAR(40) UNIQUE NOT NULL"
                    ");"
            );
            t.exec( "CREATE TABLE IF NOT EXISTS Client_phone_number ("
                    "id SERIAL PRIMARY KEY,"
                    "client_id INTEGER NOT NULL REFERENCES Client_info(id),"
                    "phone_number VARCHAR(12) UNIQUE"
                    ");"
            );
        t.commit();
        }
        catch(std::exception& ex) {
            std::cout << ex.what();
        }
    }

    void add_new_client(std::string name, std::string email) {
        std::string request {"INSERT INTO Client_info(name, email)"
                             "VALUES ('" + this->esc(name) + "','" + this->esc(email) + "');"};
        this->work(request);
    }

    void add_new_client(std::string name, std::string last_name, std::string email) {
        std::string request {"INSERT INTO Client_info(name, last_name, email)"
                             "VALUES ('" + this->esc(name) + "','" + this->esc(last_name) + "','" + this->esc(email) + "');"};
        this->work(request);
    }

    void add_client_phone(std::string email, std::string phone_number) {
        std::string request {"INSERT INTO Client_phone_number(client_id, phone_number)"
                             "VALUES ((SELECT id FROM Client_info WHERE email = '" + this->esc(email) + "'),'" + this->esc(phone_number) +"');"};
        this->work(request);
    }

    void change_info(std::string name, std::string email) {
        std::string request {"UPDATE Client_info SET name ='" + this->esc(name) + "' WHERE email = '" + this->esc(email) + "';"};
        this->work(request);
    }

    void change_info (std::string name, std::string last_name, std::string new_email, std::string old_email) {
        std::string request {"UPDATE Client_info SET name ='" + this->esc(name) + "', last_name ='"+ this->esc(last_name) + "', email ='" + this->esc(new_email) + 
                             "' WHERE email = '" + this->esc(old_email) + "';"};
        this->work(request);
    }

    void delete_phone(std::string phone) {
        std::string request {"DELETE FROM Client_phone_number "
                             "WHERE phone_number ='" + this->esc(phone) + "';"};
        this->work(request);
    }

    void delete_client(std::string email) {
        std::string request {"DELETE FROM Client_phone_number "
                             "WHERE client_id = (SELECT id FROM Client_info WHERE email ='"+ this->esc(email) +"');"
                             "DELETE FROM Client_info "
                             "WHERE email ='" + this->esc(email) + "';"
        };
        this->work(request);
    }

    pqxx::result get_info(std::string email) {
        std::string request {"SELECT name, last_name, email, phone_number FROM Client_info "
                             "JOIN Client_phone_number cpn ON cpn.client_id = (SELECT id FROM Client_info WHERE email ='" + this->esc(email) + "') "
                             "WHERE email ='" + this->esc(email) + "';"
        };
        pqxx::result result{};
        try {
            pqxx::work gi(this->get_connection());
            result = gi.exec(request);
        }
        catch (std::exception& ex) {
            std::cout << ex.what() << std::endl;
        }
        return result;
    }

    void print_info(pqxx::result result) {
       for (auto const& row: result) {
        for (auto const &field: row) std::cout << field.c_str() << '\t';
        std::cout << std::endl;
        } 
    }
};

int main() {
    Clients c;
    c.create_db_struct();
    pqxx::result info;

    c.add_new_client("Ivan", "Kosarevsky", "IvanKos@yandex.ru");
    info = c.get_info("IvanKos@yandex.ru");
    c.print_info(info);
    std::cout << "\n\n";

    c.add_client_phone("IvanKos@yandex.ru", "+79998877");
    info = c.get_info("IvanKos@yandex.ru");
    c.print_info(info);
    std::cout << "\n\n";

    c.change_info("Николай", "Белый", "NikolaBel@mail.ru", "IvanKos@yandex.ru");
    info = c.get_info("NikolaBel@mail.ru");
    c.print_info(info);
    std::cout << "\n\n";

    c.add_client_phone("NikolaBel@mail.ru", "+71112233");
    info = c.get_info("NikolaBel@mail.ru");
    c.print_info(info);
    std::cout << "\n\n";

    c.add_client_phone("NikolaBel@mail.ru", "+74442233");
    info = c.get_info("NikolaBel@mail.ru");
    c.print_info(info);
    std::cout << "\n\n";

    c.delete_phone("+74442233");
    info = c.get_info("NikolaBel@mail.ru");
    c.print_info(info);
    std::cout << "\n\n";

    c.delete_client("NikolaBel@mail.ru");
    info = c.get_info("NikolaBel@mail.ru");
    c.print_info(info);
    std::cout << "\n\n";
}