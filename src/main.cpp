#include <drogon/drogon.h>

using namespace drogon;

int main()
{
    app().addListener("0.0.0.0", 8848);
    app().run();
}