#include "input_handling.hpp"
#include <Eigen/Eigen>
#include <Eigen/src/Core/Matrix.h>

#include <chrono>
#include <cmath>
#include <cstddef>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <quill/Quill.h>

// reality can be whatever I want
constexpr float kG = 1;

// I promise I'll be careful :)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)

struct PointMassComponent {
  sf::CircleShape shape;
  float mass = 1;
  Eigen::Vector2f pos{0, 0};
  Eigen::Vector2f vel{0, 0};
};

int main() {
  quill::Config cfg;
  cfg.enable_console_colours = true;
  quill::configure(cfg);
  quill::start();
  quill::Logger* logger = quill::get_logger();
  logger->set_log_level(quill::LogLevel::Info);
  float time_rate = 1.0;

  auto const clamp_time_rate = [](float& time_rate) {
    if (time_rate < 1e-3) {
      time_rate = 1e-3;
    } else if (time_rate > 50) {
      time_rate = 50;
    }
  };
  clamp_time_rate(time_rate);

  // 'global' variables
  entt::registry registry;
  std::size_t const width = 1920;
  std::size_t const height = 1080;
  auto const window = std::make_shared<sf::RenderWindow>(
      sf::VideoMode(width, height), std::string("N-body gravity simulator"));
  window->setFramerateLimit(1000u);
  float const span = 200;
  float const aspect_ratio = 16. / 9.;
  float const x_lim_low = -span / 2;
  float const x_lim_high = span / 2;
  auto const x_span = x_lim_high - x_lim_low;
  float const y_lim_low = -span / (2 * aspect_ratio);
  float const y_lim_high = span / (2 * aspect_ratio);
  auto const y_span = y_lim_high - y_lim_low;
  auto const pixels_per_unit_length = static_cast<float>(width) / x_span;

  auto const world_to_window =
      [=](Eigen::Vector2f const& pos,
          std::shared_ptr<sf::RenderWindow> const& window) {
        float const x_interp = (pos[0] - x_lim_low) / x_span;
        float const y_interp = (pos[1] - y_lim_low) / y_span;

        auto const window_size = window->getSize();

        float const x_pos_window = x_interp * static_cast<float>(window_size.x);
        // window y axis points down, not up
        float const y_pos_window =
            (1.f - y_interp) * static_cast<float>(window_size.y);
        return Eigen::Vector2f{x_pos_window, y_pos_window};
      };

  auto const window_to_world = [=](sf::Vector2i window_coords) {
    auto const window_size = window->getSize();
    float const x_interp =
        static_cast<float>(window_coords.x) / static_cast<float>(window_size.x);
    float const x_pos = (x_span * x_interp) + x_lim_low;

    float const y_interp = 1.f - (static_cast<float>(window_coords.y) /
                                  static_cast<float>(window_size.y));
    float const y_pos = (y_span * y_interp) + y_lim_low;
    return Eigen::Vector2f{x_pos, y_pos};
  };
  auto const integrate_acceleration = [&](float const timestep) {
    auto view = registry.view<PointMassComponent>();
    std::unordered_set<entt::entity> to_delete;
    for (auto const& entity : view) {
      if (to_delete.contains(entity)) {
        continue;
      }
      auto& pm = view.get<PointMassComponent>(entity);
      auto const& point = pm.pos;

      // calculate and integrate acceleration
      Eigen::Vector2f vel_diff = Eigen::Vector2f::Zero();
      for (auto const& other_entity : view) {
        if (other_entity == entity || to_delete.contains(other_entity)) {
          // don't get acceleration from ourself
          continue;
        }
        auto const& other_pm = view.get<PointMassComponent>(other_entity);
        auto const r = pm.pos - other_pm.pos;
        auto const r_norm = r.norm();
        if (r_norm * pixels_per_unit_length <
            pm.shape.getRadius() + other_pm.shape.getRadius()) {
          // inelastic collision
          auto const sum_mass = pm.mass + other_pm.mass;
          pm.vel = (pm.mass * pm.vel + other_pm.mass * other_pm.vel) / sum_mass;
          pm.pos = (pm.mass * pm.pos + other_pm.mass * other_pm.pos) / sum_mass;
          pm.mass = sum_mass;
          float const radius = std::sqrt(pm.mass);
          pm.shape.setRadius(radius);
          pm.shape.setOrigin(radius, radius);
          to_delete.insert(other_entity);
          continue;
        }

        // add up all contributions
        vel_diff -= kG * other_pm.mass * r / std::pow(r_norm, 3.f);
      }
      // integrate
      pm.vel += vel_diff * timestep;
    }
    for (auto const& entity : to_delete) {
      registry.destroy(entity);
    }
  };
  auto const integrate_velocity_and_render = [&](float const timestep) {
    auto view = registry.view<PointMassComponent>();
    for (auto const& entity : view) {
      auto& pm = view.get<PointMassComponent>(entity);

      // integrate velocity
      pm.pos += pm.vel * timestep;

      // map coords to window coords
      auto const window_coords = world_to_window(pm.pos, window);
      pm.shape.setPosition(window_coords[0], window_coords[1]);
      window->draw(pm.shape);
    }
  };

  float const distance_limit = 1000;
  auto const cull_points = [&]() {
    auto view = registry.view<PointMassComponent>();
    for (auto const& entity : view) {
      auto const& pm = view.get<PointMassComponent>(entity);
      if (pm.pos.norm() > distance_limit) {
        registry.destroy(entity);
      }
    }
  };

  auto const spawn_point = [&](float mass, Eigen::Vector2f pos,
                               Eigen::Vector2f const& vel) {
    auto const entity = registry.create();
    sf::CircleShape shape;
    shape.setPosition(pos[0], pos[1]);
    auto const radius = std::sqrt(mass);
    shape.setRadius(radius);
    shape.setFillColor({255, 255, 255});
    shape.setOrigin(radius, radius);
    registry.emplace<PointMassComponent>(entity, std::move(shape), mass, pos,
                                         vel);
  };

  std::size_t frame_count = 0;
  auto prev_count_time = std::chrono::steady_clock::now().time_since_epoch();
  auto prev_frame_time = std::chrono::system_clock::now().time_since_epoch();

  auto const initial_spawn = [&]() {
    float const mass = 100;
    float const r = 10;
    float const v = std::sqrt(mass / r / 2);
    spawn_point(mass, {r / 2, 0}, {0, v});
    spawn_point(mass, {-r / 2, 0}, {0, -v});
  };
  initial_spawn();

  InputHandler handler(window);
  InputHandler::EventFunction const close_function = [&](sf::Event /*unused*/) {
    window->close();
  };
  handler.addCallback({sf::Event::Closed}, close_function);
  InputHandler::EventFunction const mouse_function = [&](sf::Event e) {
    static Eigen::Vector2f init_point;
    static auto init_time = std::chrono::steady_clock::now().time_since_epoch();
    auto mouse_pos = sf::Mouse::getPosition(*window);
    if (e.type == sf::Event::MouseButtonPressed) {
      init_time = std::chrono::steady_clock::now().time_since_epoch();
      init_point = window_to_world(mouse_pos);
    } else {
      float const time_diff =
          std::chrono::duration<float>(
              std::chrono::steady_clock::now().time_since_epoch() - init_time)
              .count();
      float const mass =
          std::min(100.f, std::max(1.f, time_diff * 20 * time_rate));
      auto const end_point = window_to_world(mouse_pos);
      spawn_point(mass, init_point, (end_point - init_point) * 0.25);
    }
  };
  handler.addCallback(
      {sf::Event::MouseButtonPressed, sf::Event::MouseButtonReleased},
      mouse_function);

  InputHandler::EventFunction const time_rate_function = [&](sf::Event e) {
    // TODO(henrygerardmoore): use toggleable OSD instead of debug for relevant
    // info
    if (e.key.code == sf::Keyboard::Key::Equal) {
      time_rate += 0.1;
      clamp_time_rate(time_rate);
      LOG_INFO(logger, "Time rate set to {}", time_rate);
    } else if (e.key.code == sf::Keyboard::Key::Dash) {
      time_rate -= 0.1;
      clamp_time_rate(time_rate);
      LOG_INFO(logger, "Time rate set to {}", time_rate);
    } else if (e.key.code == sf::Keyboard::Key::Num0) {
      time_rate = 1;
      clamp_time_rate(time_rate);
      LOG_INFO(logger, "Time rate set to {}", time_rate);
    } else if (e.key.code == sf::Keyboard::Key::P) {
      static float prev_time_rate = time_rate;
      if (time_rate == 0) {
        std::swap(prev_time_rate, time_rate);
      } else {
        prev_time_rate = time_rate;
        time_rate = 0;
      }
      LOG_INFO(logger, "Time rate set to {}", time_rate);
    } else if (e.key.code == sf::Keyboard::Key::R) {
      // reset spawned entities to just bistable ones
      registry.clear();
      initial_spawn();
    } else if (e.key.code == sf::Keyboard::Key::Q) {
      window->close();
    }
  };
  handler.addCallback({sf::Event::KeyPressed}, time_rate_function);
  while (window->isOpen()) {
    handler.handleEvents();
    window->clear();
    ++frame_count;
    auto const now = std::chrono::steady_clock::now().time_since_epoch();
    // NOLINTNEXTLINE
    if ((now - prev_count_time) > std::chrono::seconds(1)) {
      LOG_DEBUG(logger, "{} fps", frame_count);
      prev_count_time = now;
      frame_count = 0;
    }
    auto const timestep =
        std::max(
            std::min(
                std::chrono::duration<float>(now - prev_frame_time).count(),
                0.02f),
            0.f) *
        time_rate;
    prev_frame_time = now;
    integrate_acceleration(timestep);
    integrate_velocity_and_render(timestep);
    window->display();
    cull_points();
  }
}
// NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
