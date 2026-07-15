#include <drogon/drogon.h>

#include <cstdlib>

using namespace drogon;

int main()
{
    int port = 10000;

    if (const char* env = std::getenv("PORT"))
        port = std::stoi(env);

    app().setDocumentRoot("./public");

    app().addListener("0.0.0.0", port);

    app().run();
}