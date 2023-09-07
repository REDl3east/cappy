

template <class T>
class State {
public:
    virtual ~State() = default;
    virtual void handle_event(std::shared_ptr<T> machine, SDL_Event event) = 0;
    virtual void draw_frame(std::shared_ptr<SDL_Renderer> renderer) = 0;
};

template <class T>
class Machine : public std::enable_shared_from_this<T> {
private:
    std::unique_ptr<State<T>> currentState;

public:
    template <typename S, typename... Args>
    void set_state(Args&&... args) {
        currentState = std::make_unique<S>(std::forward<Args>(args)...);
    }

    void handle_event(SDL_Event event) {
        currentState->handle_event(std::static_pointer_cast<T>(this->shared_from_this()), event);
    }

    void draw_frame(std::shared_ptr<SDL_Renderer> renderer) {
        currentState->draw_frame(renderer);
    }
};

class CappyMachine : public Machine<CappyMachine> {
private:
    Capture& capture;

public:
    CappyMachine(Capture& c) : capture(c) {}

    Capture& get_capture() const {
        return capture;
    }
};

class MoveState : public State<CappyMachine> {
public:
    MoveState() {
        std::cout << "Entering MoveState\n";
    }

    ~MoveState() {
        std::cout << "Exiting MoveState\n";
    }

    void handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event event) override {
        std::cout << "Handling event in MoveState.\n";

    }

    void draw_frame(std::shared_ptr<SDL_Renderer> renderer) override {
        std::cout << "Drawing MoveState frame\n";
    }
};

class ColorState : public State<CappyMachine> {
public:
    ColorState() {
        std::cout << "Entering ColorState\n";
    }

    ~ColorState() {
        std::cout << "Exiting ColorState\n";
    }

    void handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event event) override {
        std::cout << "Handling event in ColorState\n";
    }

    void draw_frame(std::shared_ptr<SDL_Renderer> renderer) override {
        std::cout << "Drawing ColorState frame\n";
    }
};

class FlashlightState : public State<CappyMachine> {
public:
    FlashlightState() {
        std::cout << "Entering FlashlightState\n";
    }

    ~FlashlightState() {
        std::cout << "Exiting FlashlightState\n";
    }

    void handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event event) override {
        std::cout << "Handling event in FlashlightState\n";
    }

    void draw_frame(std::shared_ptr<SDL_Renderer> renderer) override {
        std::cout << "Drawing FlashlightState frame\n";
    }
};

class DrawCrop : public State<CappyMachine> {
public:
    DrawCrop() {
        std::cout << "Entering DrawCrop\n";
    }

    ~DrawCrop() {
        std::cout << "Exiting DrawCrop\n";
    }

    void handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event event) override {
        std::cout << "Handling event in DrawCrop\n";
    }

    void draw_frame(std::shared_ptr<SDL_Renderer> renderer) override {
        std::cout << "Drawing DrawCrop frame\n";
    }
};

class ShowCrop : public State<CappyMachine> {
public:
    ShowCrop() {
        std::cout << "Entering ShowCrop\n";
    }

    ~ShowCrop() {
        std::cout << "Exiting ShowCrop\n";
    }

    void handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event event) override {
        std::cout << "Handling event in ShowCrop\n";
    }

    void draw_frame(std::shared_ptr<SDL_Renderer> renderer) override {
        std::cout << "Drawing ShowCrop frame\n";
    }
};