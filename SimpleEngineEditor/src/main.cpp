#include <iostream>
#include <memory>
#include <SimpleEngineCore/Application.hpp>

class MyApp : public SimpleEngine::Application{
    void on_update() override{
        //std::cout << "Update frame: " << frame++ << std::endl;
    }
    int frame = 0;
};

int main(){
    auto myApp = std::make_unique<MyApp>();
    int retCode = myApp->start(1024, 768, "My application");
    return retCode;
}