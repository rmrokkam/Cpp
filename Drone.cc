#define _USE_MATH_DEFINES
#include "Drone.h"

#include <cmath>
#include <limits>

#include "AstarStrategy.h"
#include "BeelineStrategy.h"
#include "DfsStrategy.h"
#include "BfsStrategy.h"
#include "DijkstraStrategy.h"
#include "JumpDecorator.h"
#include "SpinDecorator.h"

#include "Package.h"
#include "SimulationModel.h"
#include "IPublisher.h"

Drone::Drone(JsonObject& obj) : IEntity(obj) {
  available = true;
}

Drone::~Drone() {
  if (toPackage) delete toPackage;
  if (toFinalDestination) delete toFinalDestination;
  // remove the simulation model from the list of subscribers
  this->Detach(model);
}

void Drone::getNextDelivery() {
  if (model && model->scheduledDeliveries.size() > 0) {
    package = model->scheduledDeliveries.front();
    model->scheduledDeliveries.pop_front();

    // add the simulation model to the list of subscribers
    // when we know that the model is no longer null
    this->Attach(model);

    if (package) {
      available = false;
      pickedUp = false;

      std::string message = this->getName()
                          + " is on the way to pick up "
                          + package->getName();
      this->setMessage(message);
      this->Notify();

      Vector3 packagePosition = package->getPosition();
      Vector3 finalDestination = package->getDestination();

      message = package->getName()
              + " Delivery Path: ("
              + std::to_string(packagePosition[0])
              + ", "
              + std::to_string(packagePosition[1])
              + ", "
              + std::to_string(packagePosition[2])
              + ") to (" + std::to_string(finalDestination[0])
              + ", "
              + std::to_string(finalDestination[1])
              + ", "
              + std::to_string(finalDestination[2])
              + ")";
      this->setMessage(message);
      this->Notify();

      toPackage = new BeelineStrategy(position, packagePosition);

      std::string strat = package->getStrategyName();
      if (strat == "astar") {
        toFinalDestination =
          new JumpDecorator(
            new AstarStrategy(
              packagePosition,
              finalDestination,
              model->getGraph()));
      } else if (strat == "dfs") {
        toFinalDestination =
          new SpinDecorator(
            new JumpDecorator(
              new DfsStrategy(
                packagePosition,
                finalDestination,
                model->getGraph())));
      } else if (strat == "bfs") {
        toFinalDestination =
          new SpinDecorator(
            new SpinDecorator(
              new BfsStrategy(
                packagePosition,
                finalDestination,
                model->getGraph())));
      } else if (strat == "dijkstra") {
        toFinalDestination =
          new JumpDecorator(
            new SpinDecorator(
              new DijkstraStrategy(
                packagePosition,
                finalDestination,
                model->getGraph())));
      } else {
        toFinalDestination = new BeelineStrategy(
          packagePosition,
          finalDestination);
      }
    }
  }
}

void Drone::update(double dt) {
  if (available)
    getNextDelivery();

  if (toPackage) {
    toPackage->move(this, dt);

    if (toPackage->isCompleted()) {
      delete toPackage;
      toPackage = nullptr;
      pickedUp = true;
      this->pickupPoint = package->getPosition();

      std::string message = this->getName()
                          + " has picked up "
                          + package->getName();
      this->setMessage(message);
      this->Notify();

      message = this->getName()
              + " is delivering "
              + package->getName()
              + " to the final destination.";
      this->setMessage(message);
      this->Notify();
    }
  } else if (toFinalDestination) {
    toFinalDestination->move(this, dt);

    std::string strat = package->getStrategyName();
    this->deliveryTime+=dt;

    if (package && pickedUp) {
      package->setPosition(position);
      package->setDirection(direction);
    }

    if (toFinalDestination->isCompleted()) {
      std::string message = this->getName()
                          + " has delivered "
                          + package->getName()
                          + " to the final destination.";
      this->setMessage(message);
      this->Notify();

      delete toFinalDestination;
      toFinalDestination = nullptr;

      // define data vector to add to database
      std::vector<std::string> data;
      // add the pickup and dropoff points for the package
      data.push_back(std::to_string(this->pickupPoint[0])
                    + " / " + std::to_string(this->pickupPoint[1])
                    + " / " + std::to_string(this->pickupPoint[2]));
      data.push_back(std::to_string(package->getDestination()[0])
                     + " / " + std::to_string(package->getDestination()[1])
                     + " / " + std::to_string(package->getDestination()[2]));
      // add the strategy and delivery time
      data.push_back(strat);
      data.push_back(std::to_string(deliveryTime));
      // add data vector to database
      db.putData(data);

      package->handOff();
      package = nullptr;
      available = true;
      pickedUp = false;

      this->deliveryTime = 0;
      // this->pickupPoint = Vector3();
    }
  }
}
