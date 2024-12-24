#pragma once


class Application {
    friend struct ApplicationHooks;

    void update();

public:
    Application();
    Application(const Application&) = delete;
    Application(Application&&) = delete;
} extern *currentApplication;
