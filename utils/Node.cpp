#include "Node.h"

Node::Node(int newID, double newX, double newY) {
    id = newID;
    x = newX;
    y = newY;
}

double Node::getX() const {
    return x;
}

double Node::getY() const {
    return y;
}