#include <functional>
#include <map>
#include <memory>

#include <SFML/Graphics.hpp>

class InputHandler {
 public:
  using EventFunction = std::function<void(sf::Event)>;
  using CallbackMap =
      std::map<sf::Event::EventType, std::vector<EventFunction>>;

  explicit InputHandler(std::shared_ptr<sf::Window> window);

  void addCallback(std::vector<sf::Event::EventType> const& trigger_events,
                   InputHandler::EventFunction const& function);

  void handleEvents();

 private:
  std::shared_ptr<sf::Window> window_;
  InputHandler::CallbackMap callbacks_;
};
