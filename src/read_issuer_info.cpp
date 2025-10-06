#include <iostream>
#include <libpq-fe.h>

int main() {
    // Connection string
    const char* conninfo = "dbname=finance user=douglas";

    // Connect to database
    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn);
        return 1;
    }

    std::cout << "Connected to PostgreSQL successfully!\n";

    // Execute query
    PGresult* res = PQexec(conn, "SELECT * FROM issuer_info;");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SELECT failed: " << PQerrorMessage(conn) << "\n";
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    // Print column names
    int nFields = PQnfields(res);
    for (int i = 0; i < nFields; i++) {
        std::cout << PQfname(res, i) << "\t";
    }
    std::cout << "\n-----------------------------------\n";

    // Print rows
    int nRows = PQntuples(res);
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nFields; j++) {
            std::cout << PQgetvalue(res, i, j) << "\t";
        }
        std::cout << "\n";
    }

    PQclear(res);
    PQfinish(conn);

    return 0;
}
