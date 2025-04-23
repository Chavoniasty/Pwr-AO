#ifndef Node_H
#define Node_H

class Node {
    int id;
    double x;
    double y;

   public:
    Node(int id, double newX, double newY);
    int getID() const;
    double getX() const;
    double getY() const;
};

#endif