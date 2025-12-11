// Muhammad Hashmi 24i-2037 DS-A
#include "util.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <algorithm>
using namespace std;

// constant 3 min time and screen dimensions
const int SCREEN_WIDTH = 818;
const int SCREEN_HEIGHT = 820;
const int GAME_DURATION = 180; // 3 minutes in seconds

//  classes declarations

class Passenger;
class Package;
class Building;
class Obstacle;
class Vehicle;
class GameBoard;
class Player;

class Task
{
protected:
    int pickupX, pickupY;
    int destX, destY;
    bool pickedUp;
    bool delivered;

public:
    Task(int px, int py) : pickupX(px), pickupY(py), pickedUp(false), delivered(false)
    {
        destX = 50 + rand() % (SCREEN_WIDTH - 100);
        destY = 50 + rand() % (SCREEN_HEIGHT - 100);
    }

    virtual ~Task() {}
    virtual void draw() = 0;
    virtual int getPoints() = 0;
    virtual int getPenalty() = 0;

    bool isPickedUp() const { return pickedUp; }
    bool isDelivered() const { return delivered; }
    void pickUp() { pickedUp = true; }
    void deliver() { delivered = true; }
    int getDestX() const { return destX; }
    int getDestY() const { return destY; }
    int getPickupX() const { return pickupX; }
    int getPickupY() const { return pickupY; }
};

class Passenger : public Task
{
public:
    Passenger(int px, int py) : Task(px, py) {}

    void draw() override
    {
        if (!delivered)
        {
            if (pickedUp)
            {
                DrawRoundRect(destX, destY, 40, 40, colors[LIME_GREEN]);
                DrawLine(destX + 20, destY, destX + 20, destY + 40, 5, colors[GREEN]);
            }
            else
            {
                DrawLine(pickupX, pickupY, pickupX, pickupY + 40, 5, colors[BLACK]);
            }
        }
    }

    int getPoints() override { return 10; }
    int getPenalty() override { return -5; }
};

class Package : public Task
{
public:
    Package(int px, int py) : Task(px, py) {}

    void draw() override
    {
        if (!delivered)
        {
            if (pickedUp)
            {
                DrawRoundRect(destX, destY, 40, 40, colors[LIME_GREEN]);
                DrawSquare(destX + 5, destY + 5, 30, colors[BROWN]);
            }
            else
            {
                DrawSquare(pickupX, pickupY, 40, colors[BROWN]);
            }
        }
    }

    int getPoints() override { return 20; }
    int getPenalty() override { return -8; }
};

class Building
{
protected:
    int x, y;
    int size;
    int type; // 0=normal, 1=fuel, 2=delivery, 3=role change
public:
    Building(int x, int y, int s, int t) : x(x), y(y), size(s), type(t) {}
    virtual ~Building() {}

    virtual void draw()
    {
        switch (type)
        {
        case 0: // Normal building - black
            DrawSquare(x, y, size, colors[BLACK]);
            break;
        case 1: // Fuel station - yellow
            DrawSquare(x, y, size, colors[YELLOW]);
            DrawString(x + size / 2 - 5, y + size / 2, "F", colors[BLACK]);
            break;
        case 2: // Delivery station - blue
            DrawSquare(x, y, size, colors[BLACK]);
            break;
        case 3: // Role change station - red
            DrawSquare(x, y, size, colors[RED]);
            DrawString(x + size / 2 - 5, y + size / 2, "R", colors[WHITE]);
            break;
        }
    }

    bool contains(int px, int py) const
    {
        return px >= x && px <= x + size && py >= y && py <= y + size;
    }

    bool isCollidable() const
    {
        return type == 0;
    }

    int getType() const { return type; }
    int getSize() const { return size; }
    int getX() const { return x; }
    int getY() const { return y; }
};
class Obstacle
{
protected:
    int x, y;
    int type; // 0=tree, 1=box
public:
    Obstacle(int x, int y, int t) : x(x), y(y), type(t) {}
    virtual ~Obstacle() {}

    virtual void draw()
    {
        switch (type)
        {
        case 0: // Tree
            DrawRectangle(x, y, 10, 17, colors[BROWN]);
            DrawCircle(x + 5, y + 30, 16, colors[GREEN]);
            DrawTriangle(x - 3, y, x + 5, y + 10, x + 13, y, colors[BROWN]);
            break;
        case 1: // Box
            DrawSquare(x, y, 40, colors[BLACK]);
            break;
        }
    }

    bool contains(int px, int py) const
    {
        if (type == 0)
        { // Tree
            // Check trunk collision (rectangle)
            bool inTrunk = (px >= x && px <= x + 10 && py >= y && py <= y + 17);

            // Check collision (circle)
            float distSquared = pow(px - (x + 5), 2) + pow(py - (y + 30), 2);
            bool inCanopy = distSquared <= pow(16, 2);

            return inTrunk || inCanopy;
        }
        else
        { // Boxx
            return px >= x && px <= x + 40 && py >= y && py <= y + 40;
        }
    }
};

class Vehicle
{
protected:
    int x, y;
    float *color;
    int speed;
    int fuel;
    int maxFuel;
    bool isPlayer;
    int moveCount;

public:
    Vehicle(int x, int y, float *c, bool player) : x(x), y(y), color(c), isPlayer(player), moveCount(0)
    {
        speed = 5;
        maxFuel = 400;
        fuel = maxFuel;
    }
    virtual ~Vehicle() {}
    int getSpeed() const { return speed; }

    virtual void draw() = 0;
    virtual void move(int dir)
    {
        moveCount++;
        if (moveCount % 4 == 0)
            fuel--;

        switch (dir)
        {
        case 0:
            y += speed;
            break;
        case 1:
            x += speed;
            break;
        case 2:
            y -= speed;
            break;
        case 3:
            x -= speed;
            break;
        }
    }

    void refuel() { fuel = maxFuel; }
    bool canMove() const { return fuel > 0; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getFuel() const { return fuel; }
    int getMaxFuel() const { return maxFuel; }
};

class TaxiVehicle : public Vehicle
{
public:
    TaxiVehicle(int x, int y, bool player) : Vehicle(x, y, player ? colors[ORANGE] : colors[ROYAL_BLUE], player) {}

    void draw() override
    {
        DrawCircle(x + 5, y - 2, 4, colors[BLACK]);
        DrawCircle(x + 25, y - 2, 4, colors[BLACK]);
        DrawRoundRect(x, y, 10, 13, color);
        DrawRoundRect(x + 5, y, 20, 20, color);
        DrawRoundRect(x + 8, y + 11, 5, 5, colors[BLACK]);
        DrawRoundRect(x + 20, y, 10, 13, color);
        DrawRoundRect(x + 17, y + 11, 5, 5, colors[BLACK]);
    }
};

class DeliveryVehicle : public Vehicle
{
public:
    DeliveryVehicle(int x, int y, bool player) : Vehicle(x, y, player ? colors[RED] : colors[MAGENTA], player) {}

    void draw() override
    {
        DrawCircle(x + 5, y - 2, 4, colors[BLACK]);
        DrawCircle(x + 25, y - 2, 4, colors[BLACK]);
        DrawRoundRect(x, y, 10, 13, color);
        DrawRoundRect(x + 5, y, 20, 20, color);
        DrawRoundRect(x + 8, y + 11, 5, 5, colors[BLACK]);
        DrawRoundRect(x + 20, y, 10, 13, color);
        DrawRoundRect(x + 17, y + 11, 5, 5, colors[BLACK]);
    }
};

class Player
{
private:
    string name;
    int score;
    int earnings;
    int role; // 0=taxi, 1=delivery
    Vehicle *vehicle;
    string inputName;
    bool gettingName;

public:
    Player() : name("Player"), score(0), earnings(0), role(0), gettingName(false)
    {
        vehicle = new TaxiVehicle(5, 730, true);
    }

    ~Player()
    {
        delete vehicle;
    }

    void setName(const string &n) { name = n; }

    void changeRole(int newRole)
    {
        role = newRole;
        int x = vehicle->getX();
        int y = vehicle->getY();
        delete vehicle;

        if (role == 0)
        {
            vehicle = new TaxiVehicle(x, y, true);
        }
        else
        {
            vehicle = new DeliveryVehicle(x, y, true);
        }
    }

    void addPoints(int points)
    {
        score += points;
    }
    void addEarnings(int amount)
    {
        earnings += amount;
    }
    void draw()
    {
        vehicle->draw();

        DrawRectangle(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 40, colors[WHITE]);
        DrawLine(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, SCREEN_HEIGHT - 40, 2, colors[BLACK]);

        string s_score = "Score: " + to_string(score);
        DrawString(10, SCREEN_HEIGHT - 25, s_score, colors[BLACK]);

        string s_earnings = "Earnings: $" + to_string(earnings);
        DrawString(150, SCREEN_HEIGHT - 25, s_earnings, colors[BLACK]);

        int fuelWidth = (vehicle->getFuel() * 100) / vehicle->getMaxFuel();
        DrawRectangle(380, SCREEN_HEIGHT - 30, 100, 20, colors[LIGHT_GRAY]);
        DrawRectangle(380, SCREEN_HEIGHT - 30, fuelWidth, 20, colors[GREEN]);
        DrawString(330, SCREEN_HEIGHT - 25, "Fuel", colors[BLACK]);

        string s_role = "Role: " + string(role == 0 ? "Taxi" : "Delivery");
        DrawString(500, SCREEN_HEIGHT - 25, s_role, colors[BLACK]);
    }

    void move(int direction)
    {
        vehicle->move(direction);
    }

    bool canMove() const
    {
        return vehicle->canMove();
    }

    void refuel()
    {
        if (earnings >= 5)
        {                      // Check if player has enough money
            earnings -= 5;     // Deduct $5
            vehicle->refuel(); // Call vehicle's refuel method
        }
    }

    int getRole() const { return role; }
    int getScore() const { return score; }
    int getX() const { return vehicle->getX(); }
    int getY() const { return vehicle->getY(); }
    string getName() const { return name; }

    void startNameInput()
    {
        gettingName = true;
        inputName.clear();
    }

    void addToName(char c)
    {
        if (gettingName)
        {
            if (c == 13)
            { // Enter
                name = inputName.empty() ? "Player" : inputName;
                gettingName = false;
            }
            else if (c == 8)
            { // Backspace
                if (!inputName.empty())
                    inputName.pop_back();
            }
            else if (isalnum(c) || c == ' ')
            {
                if (inputName.length() < 15)
                    inputName += c;
            }
        }
    }

    bool isGettingName() const { return gettingName; }

    void drawNameInput()
    {
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        string prompt = "Enter your name (Press Enter when done):";
        DrawString(250, 450, prompt, colors[BLACK]);

        string nameDisplay = inputName;
        if ((glutGet(GLUT_ELAPSED_TIME) / 500) % 2 == 0)
        {
            nameDisplay += "_";
        }
        DrawString(300, 400, nameDisplay, colors[BLACK]);
    }
};

class GameBoard
{
private:
    Building *buildings[20];
    Obstacle *obstacles[20];
    Vehicle *otherVehicles[5];
    Task *tasks[4];
    int buildingCount;
    int obstacleCount;
    int vehicleCount;
    int taskCount;
    int timeLeft;
    int frameCount;

public:
    GameBoard() : buildingCount(0), obstacleCount(0), vehicleCount(0), taskCount(0), timeLeft(GAME_DURATION), frameCount(0)
    {
        generateBoard();
    }

    ~GameBoard()
    {
        clearAll();
    }

    void addNewTask(int taskType)
    {
        if (taskCount < 4)
        {
            int x, y;
            bool validPosition = false;
            int attempts = 0;
            const int MAX_ATTEMPTS = 200; // Increased attempts for better placement of th
            //  new generated task

            // Task dimensions
            const int TASK_WIDTH = 40;
            const int TASK_HEIGHT = 40;

            while (!validPosition && attempts < MAX_ATTEMPTS)
            {
                // Generate position in one of the four quadrants
                int quadrant = rand() % 4;
                switch (quadrant)
                {
                case 0: // Top-left
                    x = 50 + rand() % (SCREEN_WIDTH / 2 - 100);
                    y = SCREEN_HEIGHT / 2 + rand() % (SCREEN_HEIGHT / 2 - 100);
                    break;
                case 1: // Top-right
                    x = SCREEN_WIDTH / 2 + rand() % (SCREEN_WIDTH / 2 - 100);
                    y = SCREEN_HEIGHT / 2 + rand() % (SCREEN_HEIGHT / 2 - 100);
                    break;
                case 2: // Bottom-left
                    x = 50 + rand() % (SCREEN_WIDTH / 2 - 100);
                    y = 50 + rand() % (SCREEN_HEIGHT / 2 - 100);
                    break;
                case 3: // Bottom-right
                    x = SCREEN_WIDTH / 2 + rand() % (SCREEN_WIDTH / 2 - 100);
                    y = 50 + rand() % (SCREEN_HEIGHT / 2 - 100);
                    break;
                }

                validPosition = true;

                // Check all four corners of the task
                int corners[4][2] = {
                    {x, y}, {x + TASK_WIDTH, y}, {x, y + TASK_HEIGHT}, {x + TASK_WIDTH, y + TASK_HEIGHT}};

                // Check against buildings
                for (int i = 0; i < buildingCount && validPosition; i++)
                {
                    int bx = buildings[i]->getX();
                    int by = buildings[i]->getY();
                    int bsize = buildings[i]->getSize();

                    // Check if any corner is inside the building
                    for (int c = 0; c < 4; c++)
                    {
                        if (corners[c][0] >= bx && corners[c][0] <= bx + bsize &&
                            corners[c][1] >= by && corners[c][1] <= by + bsize)
                        {
                            validPosition = false;
                            break;
                        }
                    }
                }

                // Check against obstacles
                for (int i = 0; i < obstacleCount && validPosition; i++)
                {
                    // Check if any corner collides with obstacle
                    for (int c = 0; c < 4; c++)
                    {
                        if (obstacles[i]->contains(corners[c][0], corners[c][1]))
                        {
                            validPosition = false;
                            break;
                        }
                    }
                }

                // Check against other tasks
                for (int i = 0; i < taskCount && validPosition; i++)
                {
                    int tx = tasks[i]->getPickupX();
                    int ty = tasks[i]->getPickupY();

                    // Minimum distance between tasks
                    if (abs(x - tx) < 80 && abs(y - ty) < 80)
                    {
                        validPosition = false;
                    }
                }

                // Check against vehicles
                for (int i = 0; i < vehicleCount && validPosition; i++)
                {
                    int vx = otherVehicles[i]->getX();
                    int vy = otherVehicles[i]->getY();

                    // Simple rectangle collision check
                    if (x < vx + 30 && x + TASK_WIDTH > vx &&
                        y < vy + 30 && y + TASK_HEIGHT > vy)
                    {
                        validPosition = false;
                    }
                }

                attempts++;
            }

            if (validPosition)
            {
                if (taskType == 0)
                {
                    tasks[taskCount++] = new Passenger(x, y);
                }
                else
                {
                    tasks[taskCount++] = new Package(x, y);
                }
            }
        }
    }
    void removeDeliveredTasks()
    {
        for (int i = 0; i < taskCount; i++)
        {
            if (tasks[i]->isDelivered())
            {
                delete tasks[i];
                // Shift remaining tasks
                for (int j = i; j < taskCount - 1; j++)
                {
                    tasks[j] = tasks[j + 1];
                }
                taskCount--;
                i--;
            }
        }
    }
    void clearAll()
    {
        for (int i = 0; i < buildingCount; i++)
        {
            if (buildings[i])
                delete buildings[i];
        }
        for (int i = 0; i < obstacleCount; i++)
        {
            if (obstacles[i])
                delete obstacles[i];
        }
        for (int i = 0; i < vehicleCount; i++)
        {
            if (otherVehicles[i])
                delete otherVehicles[i];
        }
        for (int i = 0; i < taskCount; i++)
        {
            if (tasks[i])
                delete tasks[i];
        }
        buildingCount = obstacleCount = vehicleCount = taskCount = 0;
    }

    void generateBoard()
    {
        clearAll();

        const int BUILDING_SIZE = 40;
        const int OBSTACLE_SIZE = 40;
        const int BOARD_AREA = (SCREEN_WIDTH) * (SCREEN_HEIGHT - 40);
        const int TARGET_AREA = BOARD_AREA * 0.35;
        int current_area = 0;

        // Special buildings - 3 fuel stations and 1 role change station
        buildings[buildingCount++] = new Building(10, 10, BUILDING_SIZE, 1);                                        // Fuel station 1 (bottom-left)
        buildings[buildingCount++] = new Building(SCREEN_WIDTH - 50, SCREEN_HEIGHT - 90, BUILDING_SIZE, 1);         // Fuel station 2 (top-right)
        buildings[buildingCount++] = new Building(SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 20, BUILDING_SIZE, 1); // Fuel station 3 (center)
        buildings[buildingCount++] = new Building(SCREEN_WIDTH - 50, 10, BUILDING_SIZE, 3);                         // Role change station

        // Normal buildings
        while (current_area < TARGET_AREA && buildingCount < 20)
        {
            int x = 50 + rand() % (SCREEN_WIDTH - 100 - BUILDING_SIZE);
            int y = 50 + rand() % (SCREEN_HEIGHT - 100 - BUILDING_SIZE);

            bool validPosition = true;

            for (int i = 0; i < buildingCount; i++)
            {
                int bx = buildings[i]->getX();
                int by = buildings[i]->getY();
                int bsize = buildings[i]->getSize();

                if (x < bx + bsize && x + BUILDING_SIZE > bx &&
                    y < by + bsize && y + BUILDING_SIZE > by)
                {
                    validPosition = false;
                    break;
                }
            }

            if (validPosition)
            {
                buildings[buildingCount++] = new Building(x, y, BUILDING_SIZE, 0);
                current_area += BUILDING_SIZE * BUILDING_SIZE;
            }
        }

        // Obstacles
        while (obstacleCount < 20)
        {
            int x = 50 + rand() % (SCREEN_WIDTH - 100 - OBSTACLE_SIZE);
            int y = 50 + rand() % (SCREEN_HEIGHT - 100 - OBSTACLE_SIZE);

            bool validPosition = true;

            for (int i = 0; i < buildingCount; i++)
            {
                int bx = buildings[i]->getX();
                int by = buildings[i]->getY();
                int bsize = buildings[i]->getSize();

                if (x < bx + bsize && x + OBSTACLE_SIZE > bx &&
                    y < by + bsize && y + OBSTACLE_SIZE > by)
                {
                    validPosition = false;
                    break;
                }
            }

            if (validPosition)
            {
                int type = rand() % 2;
                obstacles[obstacleCount++] = new Obstacle(x, y, type);
            }
        }

        // NPC Vehicles now with random types
        vehicleCount = 2 + rand() % 3; // 2-4 vehicles
        for (int i = 0; i < vehicleCount; i++)
        {
            int x, y;
            bool validPosition;
            int attempts = 0;

            do
            {
                validPosition = true;
                x = 100 + rand() % (SCREEN_WIDTH - 200);
                y = 100 + rand() % (SCREEN_HEIGHT - 200);

                for (int j = 0; j < buildingCount; j++)
                {
                    int bx = buildings[j]->getX();
                    int by = buildings[j]->getY();
                    int bsize = buildings[j]->getSize();

                    if (x < bx + bsize && x + 30 > bx &&
                        y < by + bsize && y + 30 > by)
                    {
                        validPosition = false;
                        break;
                    }
                }

                // Check obstacles
                for (int j = 0; j < obstacleCount && validPosition; j++)
                {
                    if (obstacles[j]->contains(x, y) ||
                        obstacles[j]->contains(x + 30, y) ||
                        obstacles[j]->contains(x, y + 30) ||
                        obstacles[j]->contains(x + 30, y + 30))
                    {
                        validPosition = false;
                        break;
                    }
                }

                attempts++;
                if (attempts > 50)
                    break;

            } while (!validPosition);

            if (validPosition)
            {
                // 50/50 chance for either vehicle type
                if (rand() % 2 == 0)
                {
                    otherVehicles[i] = new TaxiVehicle(x, y, false);
                }
                else
                {
                    otherVehicles[i] = new DeliveryVehicle(x, y, false);
                }
            }
            else
            {
                vehicleCount--;
                i--;
            }
        }

        // Tasks
        for (int i = 0; i < 2 && taskCount < 4; i++)
        {
            int x = 100 + rand() % (SCREEN_WIDTH - 200);
            int y = 100 + rand() % (SCREEN_HEIGHT - 200);
            tasks[taskCount++] = new Passenger(x, y);
        }

        for (int i = 0; i < 2 && taskCount < 4; i++)
        {
            int x = 100 + rand() % (SCREEN_WIDTH - 200);
            int y = 100 + rand() % (SCREEN_HEIGHT - 200);
            tasks[taskCount++] = new Package(x, y);
        }
    }
    void draw(int playerRole)
    {
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 40, colors[WHITE]);
        DrawLine(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, SCREEN_HEIGHT - 40, 4, colors[BLACK]);
        DrawLine(0, 0, 0, SCREEN_HEIGHT - 40, 4, colors[BLACK]);
        DrawLine(SCREEN_WIDTH - 2, 0, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 40, 4, colors[BLACK]);
        DrawLine(0, 2, SCREEN_WIDTH, 2, 4, colors[BLACK]);

        for (int i = 0; i < buildingCount; i++)
            buildings[i]->draw();
        for (int i = 0; i < obstacleCount; i++)
            obstacles[i]->draw();
        for (int i = 0; i < vehicleCount; i++)
            otherVehicles[i]->draw();

        for (int i = 0; i < taskCount; i++)
        {
            if ((playerRole == 0 && dynamic_cast<Passenger *>(tasks[i])) ||
                (playerRole == 1 && dynamic_cast<Package *>(tasks[i])))
            {
                tasks[i]->draw();
            }
        }
    }

    int checkCollisionType(int x, int y)
    {
        // Check vehicle corners
        int vehicleCorners[4][2] = {
            {x, y},          // top-left
            {x + 30, y},     // top-right
            {x, y + 30},     // bottom-left
            {x + 30, y + 30} // bottom-right
        };

        /* // Check collision with people (tasks)
         for (int i = 0; i < taskCount; i++) {
             if (!tasks[i]->isPickedUp() && !tasks[i]->isDelivered()) {
                 int px = tasks[i]->getPickupX();
                 int py = tasks[i]->getPickupY();
                 for (int c = 0; c < 4; c++) {
                     if (abs(vehicleCorners[c][0] - px) < 30 && abs(vehicleCorners[c][1] - py) < 30) {
                         return 1; // Collision with person
                     }
                 }
             }
         } */

        // Check collision with collidable buildings
        for (int i = 0; i < buildingCount; i++)
        {
            if (buildings[i]->isCollidable())
            {
                for (int c = 0; c < 4; c++)
                {
                    if (buildings[i]->contains(vehicleCorners[c][0], vehicleCorners[c][1]))
                    {
                        return 2; // Collision with building (treated as obstacle)
                    }
                }
            }
        }

        // Check collision with obstacles
        for (int i = 0; i < obstacleCount; i++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (obstacles[i]->contains(vehicleCorners[c][0], vehicleCorners[c][1]))
                {
                    return 2; // Collision with obstacle
                }
            }
        }

        // Check collision with other vehicles
        for (int i = 0; i < vehicleCount; i++)
        {
            int vx = otherVehicles[i]->getX();
            int vy = otherVehicles[i]->getY();

            // Simple rectangle collision
            if (x < vx + 30 && x + 30 > vx &&
                y < vy + 30 && y + 30 > vy)
            {
                return 3; // Collision with other vehicle
            }
        }

        return 0; // No collision
    }

    Task *checkTaskPickup(int x, int y, int role)
    {
        for (int i = 0; i < taskCount; i++)
        {
            if ((role == 0 && dynamic_cast<Passenger *>(tasks[i])) ||
                (role == 1 && dynamic_cast<Package *>(tasks[i])))
            {

                int px = tasks[i]->getPickupX();
                int py = tasks[i]->getPickupY();

                if (!tasks[i]->isPickedUp() && abs(x - px) < 30 && abs(y - py) < 30)
                {
                    tasks[i]->pickUp();
                    return tasks[i];
                }
            }
        }
        return nullptr;
    }

    int checkTaskDelivery(int x, int y, Task *currentTask)
    {
        if (currentTask && currentTask->isPickedUp() && !currentTask->isDelivered())
        {
            int dx = currentTask->getDestX();
            int dy = currentTask->getDestY();
            if (abs(x - dx) < 30 && abs(y - dy) < 30)
            {
                currentTask->deliver();
                // Return 0 for passenger, 1 for package
                return dynamic_cast<Passenger *>(currentTask) ? 0 : 1;
            }
        }
        return -1; // No delivery
    }
    bool checkFuelStation(int x, int y)
    {
        // Check all 5 positions (current + 4 adjacent)
        int checkPositions[5][2] = {
            {x, y},      // current position
            {x + 30, y}, // right
            {x, y + 30}, // bottom
            {x - 30, y}, // left
            {x, y - 30}  // top
        };

        for (int i = 0; i < buildingCount; i++)
        {
            if (buildings[i]->getType() == 1)
            { // Fuel station
                for (int j = 0; j < 5; j++)
                {
                    if (buildings[i]->contains(checkPositions[j][0], checkPositions[j][1]))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    bool checkRoleChangeStation(int x, int y)
    {
        for (int i = 0; i < buildingCount; i++)
        {
            if (buildings[i]->getType() == 3 && buildings[i]->contains(x, y))
            {
                return true;
            }
        }
        return false;
    }
    void moveOtherVehicles()
    {
        static int lastDirectionChange[5] = {0};
        static int vehicleDirections[5];

        for (int i = 0; i < vehicleCount; i++)
        {
            if (lastDirectionChange[i] == 0)
            {
                vehicleDirections[i] = rand() % 4;
                lastDirectionChange[i] = 30 + rand() % 30;
            }

            if (--lastDirectionChange[i] <= 0)
            {
                vehicleDirections[i] = rand() % 4;
                lastDirectionChange[i] = 30 + rand() % 30;
            }

            int dir = vehicleDirections[i];
            int currentX = otherVehicles[i]->getX();
            int currentY = otherVehicles[i]->getY();
            int newX = currentX;
            int newY = currentY;

            switch (dir)
            {
            case 0:
                newY += otherVehicles[i]->getSpeed();
                break;
            case 1:
                newX += otherVehicles[i]->getSpeed();
                break;
            case 2:
                newY -= otherVehicles[i]->getSpeed();
                break;
            case 3:
                newX -= otherVehicles[i]->getSpeed();
                break;
            }

            bool canMove = true;
            if (newX < 10 || newX > SCREEN_WIDTH - 40 ||
                newY < 10 || newY > SCREEN_HEIGHT - 70)
            {
                canMove = false;
            }

            if (canMove)
            {
                // Check all four corners of the vehicle
                int corners[4][2] = {
                    {newX, newY}, {newX + 30, newY}, {newX, newY + 30}, {newX + 30, newY + 30}};

                // Check buildings but only collidable ones
                for (int j = 0; j < buildingCount && canMove; j++)
                {
                    if (buildings[j]->isCollidable())
                    {
                        for (int c = 0; c < 4 && canMove; c++)
                        {
                            if (buildings[j]->contains(corners[c][0], corners[c][1]))
                            {
                                canMove = false;
                            }
                        }
                    }
                }

                // Check obstacles
                for (int j = 0; j < obstacleCount && canMove; j++)
                {
                    for (int c = 0; c < 4 && canMove; c++)
                    {
                        if (obstacles[j]->contains(corners[c][0], corners[c][1]))
                        {
                            canMove = false;
                        }
                    }
                }

                // Check other vehicles
                for (int j = 0; j < vehicleCount && canMove; j++)
                {
                    if (i != j)
                    {
                        int vx = otherVehicles[j]->getX();
                        int vy = otherVehicles[j]->getY();
                        if (newX < vx + 30 && newX + 30 > vx &&
                            newY < vy + 30 && newY + 30 > vy)
                        {
                            canMove = false;
                        }
                    }
                }
            }

            if (!canMove)
            {
                vehicleDirections[i] = rand() % 4;
                lastDirectionChange[i] = 30 + rand() % 30;
            }
            else
            {
                otherVehicles[i]->move(dir);
            }
        }
    }
    void updateTime()
    {
        frameCount++;
        if (frameCount % 60 == 0 && timeLeft > 0)
            timeLeft--;
    }

    int getTimeLeft() const { return timeLeft; }
    void setTimeLeft(int time) { timeLeft = time; }

    void addNewVehicle()
    {
        if (vehicleCount < 5)
        {
            int x = 100 + rand() % (SCREEN_WIDTH - 200);
            int y = 100 + rand() % (SCREEN_HEIGHT - 200);
            if (rand() % 2 == 0)
            {
                otherVehicles[vehicleCount++] = new TaxiVehicle(x, y, false);
            }
            else
            {
                otherVehicles[vehicleCount++] = new DeliveryVehicle(x, y, false);
            }
        }
    }
};

class GameManager
{
private:
    Player *player;
    GameBoard *board;
    Task *currentTask;
    int gameState;
    string leaderboard[10];
    int scores[10];
    int selectedRole;
    time_t startTime;
    bool gameWon;
    void loadLeaderboard()
    {
        ifstream file("highscores.txt");
        if (file.is_open())
        {
            for (int i = 0; i < 10; i++)
            {
                string line;
                if (getline(file, line))
                {
                    size_t spacePos = line.find(' ');
                    if (spacePos != string::npos)
                    {
                        leaderboard[i] = line.substr(0, spacePos);
                        try
                        {
                            scores[i] = stoi(line.substr(spacePos + 1));
                        }
                        catch (...)
                        {
                            scores[i] = 0;
                        }
                    }
                    else
                    {
                        leaderboard[i] = "";
                        scores[i] = 0;
                    }
                }
                else
                {
                    leaderboard[i] = "";
                    scores[i] = 0;
                }
            }
            file.close();
        }
        else
        {
            // Initialize empty leaderboard if file doesn't exist
            for (int i = 0; i < 10; i++)
            {
                leaderboard[i] = "";
                scores[i] = 0;
            }
        }
    }

    void saveLeaderboard()
    {
        ofstream file("highscores.txt");
        if (file.is_open())
        {
            for (int i = 0; i < 10; i++)
            {
                if (!leaderboard[i].empty())
                {
                    file << leaderboard[i] << " " << scores[i] << endl;
                }
            }
            file.close();
        }
    }
    void updateLeaderboard()
    {
        int playerScore = player->getScore();
        for (int i = 0; i < 10; i++)
        {
            if (leaderboard[i].empty() || playerScore > scores[i])
            {
                for (int j = 9; j > i; j--)
                {
                    leaderboard[j] = leaderboard[j - 1];
                    scores[j] = scores[j - 1]; // move each score one step low
                }
                leaderboard[i] = player->getName();
                scores[i] = playerScore; // add input score in the
                // place of the one it was greater than
                break;
            }
        }
        saveLeaderboard();
    }

public:
    GameManager() : player(nullptr), board(nullptr), currentTask(nullptr),
                    gameState(0), selectedRole(0), gameWon(false)
    {
        loadLeaderboard();
    }

    ~GameManager()
    {
        delete player;
        delete board;
    }

    void startGame()
    {
        if (!player)
            player = new Player();
        player->changeRole(selectedRole);
        if (board)
            delete board;
        board = new GameBoard();
        gameState = 1;
        startTime = time(0);
        gameWon = false;
    }

    void endGame()
    {
        if (player && player->getScore() > 0)
            updateLeaderboard();
        gameState = 4;
    }

    void drawMenu()
    {
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawString(255, 570, "WELCOME TO 🚗 RUSH HOUR 🚕", colors[BLACK]);
        DrawString(275, 470, "For Leader Board Press 'L'", colors[BLACK]);
        DrawString(70, 270, "For Taxi Mode Press 'T'", colors[BLACK]);
        DrawString(500, 270, "For Delivery Mode Press 'D'", colors[BLACK]);
        DrawString(290, 170, "For Random Role Press 'X'", colors[BLACK]);
    }

    void drawLeaderboard()
    {
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawString(350, 750, "LEADERBOARD", colors[BLACK]);

        int position = 1; // track actual position numbers
        int yPos = 700;   // Starting Y position

        for (int i = 0; i < 10; i++)
        {
            if (!leaderboard[i].empty())
            {
                string entry = to_string(position) + ". " + leaderboard[i] + " - " + to_string(scores[i]);
                DrawString(300, yPos, entry, colors[BLACK]);
                position++; // Only increment for non-empty entries
                yPos -= 50; // Move up for next entry
            }
        }

        DrawString(300, 200, "Press ENTER to go back", colors[BLACK]);
    }

    void drawGame()
    {
        if (board && player)
        {
            board->draw(player->getRole());
            player->draw();

            int minutes = board->getTimeLeft() / 60;
            int seconds = board->getTimeLeft() % 60;
            string timeText = "Time: " + to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + to_string(seconds);
            DrawString(660, SCREEN_HEIGHT - 25, timeText, colors[BLACK]);
        }
    }

    void drawNameInput()
    {
        player->drawNameInput();
    }

    void drawGameOver()
    {
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        string gameOverText = gameWon ? "YOU WON!" : "GAME OVER";
        DrawString(350, 500, gameOverText, colors[BLACK]);

        string scoreText = "Final Score: " + to_string(player->getScore());
        DrawString(350, 450, scoreText, colors[BLACK]);

        string restartText = "Press ENTER to return to menu";
        DrawString(300, 400, restartText, colors[BLACK]);
    }

    void draw()
    {
        switch (gameState)
        {
        case 0:
            drawMenu();
            break;
        case 1:
            drawGame();
            break;
        case 2:
            drawLeaderboard();
            break;
        case 3:
            drawNameInput();
            break;
        case 4:
            drawGameOver();
            break;
        }
    }

    void handleKey(unsigned char key, int x, int y)
    {
        if (gameState == 0)
        {
            if (key == 't' || key == 'T')
            {
                selectedRole = 0;
                if (!player)
                    player = new Player();
                player->startNameInput();
                gameState = 3;
            }
            else if (key == 'd' || key == 'D')
            {
                selectedRole = 1;
                if (!player)
                    player = new Player();
                player->startNameInput();
                gameState = 3;
            }
            else if (key == 'x' || key == 'X')
            {
                selectedRole = rand() % 2;
                if (!player)
                    player = new Player();
                player->startNameInput();
                gameState = 3;
            }
            else if (key == 'l' || key == 'L')
            {
                gameState = 2;
            }
        }
        else if (gameState == 1)
        {
            if (key == ' ')
            {
                if (!currentTask)
                {
                    currentTask = board->checkTaskPickup(player->getX(), player->getY(), player->getRole());
                }
                else if (currentTask->isPickedUp() && !currentTask->isDelivered())
                {
                    int deliveryResult = board->checkTaskDelivery(player->getX(), player->getY(), currentTask);

                    // Check if a delivery was completed
                    if (deliveryResult != -1)
                    {
                        bool wasPassenger = dynamic_cast<Passenger *>(currentTask) != nullptr;
                        player->addPoints(currentTask->getPoints());
                        int fare = 8 + rand() % 8;
                        player->addEarnings(fare);

                        // Remove delivered tasks and spawn new one
                        board->removeDeliveredTasks();
                        board->addNewTask(wasPassenger ? 0 : 1);

                        currentTask = nullptr;
                    }
                }
            }
            else if (key == 'r' || key == 'R')
            {
                if (board && player && board->checkRoleChangeStation(player->getX(), player->getY()))
                {
                    player->changeRole(1 - player->getRole());
                    currentTask = nullptr;
                }
            }
            else if (key == 'f' || key == 'F')
            {
                if (board && player && board->checkFuelStation(player->getX(), player->getY()))
                {
                    player->refuel();
                }
            }
        }
        else if (gameState == 2)
        {
            if (key == 13)
                gameState = 0;
        }
        else if (gameState == 3)
        {
            player->addToName(key);
            if (!player->isGettingName())
                startGame();
        }
        else if (gameState == 4)
        {
            if (key == 13)
            {
                gameState = 0;
                delete player;
                delete board;
                player = nullptr;
                board = nullptr;
                currentTask = nullptr;
            }
        }
    }
    void handleSpecialKey(int key, int x, int y)
    {
        if (gameState == 1 && player && player->canMove())
        {
            int direction = -1;
            switch (key)
            {
            case GLUT_KEY_UP:
                direction = 0;
                break;
            case GLUT_KEY_RIGHT:
                direction = 1;
                break;
            case GLUT_KEY_DOWN:
                direction = 2;
                break;
            case GLUT_KEY_LEFT:
                direction = 3;
                break;
            }

            if (direction != -1)
            {
                int newX = player->getX();
                int newY = player->getY();

                // Calculate new position based on direction
                switch (direction)
                {
                case 0:
                    newY += 5;
                    break;
                case 1:
                    newX += 5;
                    break;
                case 2:
                    newY -= 5;
                    break;
                case 3:
                    newX -= 5;
                    break;
                }

                // Check boundaries
                bool outsideBoundaries = false;
                int vehicleWidth = 30;
                int vehicleHeight = 30;
                int margin = 5;

                int leftBoundary = margin;
                int rightBoundary = SCREEN_WIDTH - vehicleWidth - margin;
                int bottomBoundary = margin;
                int topBoundary = SCREEN_HEIGHT - vehicleHeight - 40 - margin;

                if (newX < leftBoundary || newX > rightBoundary ||
                    newY < bottomBoundary || newY > topBoundary)
                {
                    outsideBoundaries = true;
                }

                if (!outsideBoundaries)
                {
                    int collisionType = board->checkCollisionType(newX, newY);
                    if (collisionType == 0)
                    {
                        player->move(direction);
                    }
                    else
                    {
                        // Apply appropriate penalty based on role and collision type
                        int penalty = 0;
                        if (player->getRole() == 0)
                        { // Taxi wala
                            switch (collisionType)
                            {
                            case 1:
                                penalty = -5;
                                break; // Person but removed because it was not picking up
                            case 2:
                                penalty = -2;
                                break; // Obstacle
                            case 3:
                                penalty = -3;
                                break; // Other vehicle
                            }
                        }
                        else
                        { // Delivery wala foodpanda
                            switch (collisionType)
                            {
                            case 1:
                                penalty = -8;
                                break; // Person
                            case 2:
                                penalty = -4;
                                break; // Obstacle
                            case 3:
                                penalty = -5;
                                break; // Other vehicle
                            }
                        }
                        player->addPoints(penalty);
                    }
                }
            }
        }
    }
    void update()
    {
        if (gameState == 1 && board && player)
        {
            board->moveOtherVehicles();

            time_t currentTime = time(0);
            int elapsed = difftime(currentTime, startTime);
            board->setTimeLeft(max(0, GAME_DURATION - elapsed));

            if (board->getTimeLeft() <= 0)
            {
                gameWon = (player->getScore() >= 100);
                endGame();
            }
            else if (player->getScore() < 0 || !player->canMove())
            {
                gameWon = false;
                endGame();
            }
            else if (player->getScore() >= 100)
            {
                gameWon = true;
                endGame();
            }
        }
    }

    int getGameState() const { return gameState; }
    GameBoard *getBoard() { return board; }
};

// Global game manager instance
GameManager gameManager;

void Initialize()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GameDisplay()
{
    gameManager.draw();
    glutSwapBuffers();
}

void NonPrintableKeys(int key, int x, int y)
{
    gameManager.handleSpecialKey(key, x, y);
    glutPostRedisplay();
}

void PrintableKeys(unsigned char key, int x, int y)
{
    if (key == 27)
    { // ASCII code for Escape
        exit(0);
    }
    gameManager.handleKey(key, x, y);
    glutPostRedisplay();
}

void Timer(int m)
{
    gameManager.update();
    glutPostRedisplay();
    glutTimerFunc(1000 / 60, Timer, 0);
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("Rush Hour Game ");

    Initialize();

    glutDisplayFunc(GameDisplay);
    glutSpecialFunc(NonPrintableKeys);
    glutKeyboardFunc(PrintableKeys);
    glutTimerFunc(1000 / 60, Timer, 0);

    glutMainLoop();
    return 0;
}