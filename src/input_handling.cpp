#include "input_handling.hpp"

InputHandler::InputHandler(std::shared_ptr<sf::Window> window)
    : window_(std::move(window)) {}

void InputHandler::addCallback(
    std::vector<sf::Event::EventType> const& trigger_events,
    EventFunction const& function) {
  for (auto const& trigger_event : trigger_events) {
    callbacks_[trigger_event].push_back(function);
  }
}

void InputHandler::handleEvents() {
  for (auto event = sf::Event{}; window_->pollEvent(event);) {
    for (auto const& func : callbacks_[event.type]) {
      func(event);
    }
  }
}
