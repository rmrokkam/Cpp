#include "IPublisher.h"

#include <iostream>
#include <algorithm>

void IPublisher::Attach(ISubscriber* subscriber) {
    subscribers.push_back(subscriber);
}

void IPublisher::Detach(ISubscriber* subscriber) {
    std::vector<ISubscriber*>::iterator it = std::find(subscribers.begin(),
                                                       subscribers.end(),
                                                       subscriber);
    subscribers.erase(it);
}

void IPublisher::Notify() {
    for (int i=0; i < subscribers.size(); i++) {
        if (subscribers.empty()) {
            std::cout << "list of subscribers is empty" << std::endl;
            return;
        }
        if (subscribers.at(i) == nullptr) {
            std::cout << "model is null" << std::endl;
            return;
        }
        subscribers.at(i)->receiveMessage(message);
    }
}

void IPublisher::setMessage(std::string& messageString) {
    this->message = messageString;
}
