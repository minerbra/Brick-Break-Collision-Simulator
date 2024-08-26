#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
    float red, green, blue;
    float x, y, width;
    BRICKTYPE brick_type;
    ONOFF onoff;
    int hitCount; // Number of times the brick has been hit

    Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
    {
        brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
        onoff = ON;
        hitCount = 0;
    };

    void drawBrick()
    {
        if (onoff == ON)
        {
            double halfside = width / 2;

            glColor3d(red, green, blue);
            glBegin(GL_POLYGON);

            glVertex2d(x + halfside, y + halfside);
            glVertex2d(x + halfside, y - halfside);
            glVertex2d(x - halfside, y - halfside);
            glVertex2d(x - halfside, y + halfside);

            glEnd();
        }
    }

    void changeColor()
    {
        red = static_cast<float>(rand()) / RAND_MAX;
        green = static_cast<float>(rand()) / RAND_MAX;
        blue = static_cast<float>(rand()) / RAND_MAX;
    }

    void handleCollision()
    {
        if (hitCount < 2)
        {
            hitCount++;
            changeColor();
        }
        else
        {
            onoff = OFF;
        }
    }
};

class Circle
{
public:
    float red, green, blue;
    float radius;
    float x;
    float y;
    float speed = 0.01; // Slower speed
    int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

    Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
    {
        x = xx;
        y = yy;
        radius = rr;
        red = r;
        green = g;
        blue = b;
        direction = dir;
    }

    void CheckCollision(Brick* brk)
    {
        if (brk->brick_type == DESTRUCTABLE && brk->onoff == ON)
        {
            bool collision = (x + radius > brk->x - brk->width && x - radius < brk->x + brk->width &&
                y + radius > brk->y - brk->width && y - radius < brk->y + brk->width);
            if (collision)
            {
                brk->handleCollision();

                // Resolve collision: move the ball out of the brick
                float overlapX = min(radius, (brk->x + brk->width) - x) - min(radius, x - (brk->x - brk->width));
                float overlapY = min(radius, (brk->y + brk->width) - y) - min(radius, y - (brk->y - brk->width));

                if (abs(overlapX) < abs(overlapY))
                {
                    // Horizontal collision
                    if (direction == 2 || direction == 7 || direction == 8) // right or down-right or down-left
                        x -= overlapX;
                    else if (direction == 4 || direction == 5 || direction == 6) // left or up-left or up-right
                        x += overlapX;
                    direction = (direction == 2 || direction == 4) ? ((rand() % 2 == 0) ? 1 : 3) : ((rand() % 2 == 0) ? 7 : 8);
                }
                else
                {
                    // Vertical collision
                    if (direction == 1 || direction == 5 || direction == 6) // up or up-right or up-left
                        y += overlapY;
                    else if (direction == 3 || direction == 7 || direction == 8) // down or down-right or down-left
                        y -= overlapY;
                    direction = (direction == 1 || direction == 3) ? ((rand() % 2 == 0) ? 2 : 4) : ((rand() % 2 == 0) ? 7 : 8);
                }
            }
        }
    }

    void CheckCircleCollision(vector<Circle>& world, vector<Circle>& toRemove)
    {
        for (auto& other : world)
        {
            if (&other != this) // Check against other circles
            {
                float dx = x - other.x;
                float dy = y - other.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < radius + other.radius)
                {
                    // Add to remove list
                    toRemove.push_back(other);
                    // Mark this circle for removal as well
                    toRemove.push_back(*this);
                }
            }
        }
    }

    void MoveOneStep()
    {
        if (direction == 1 || direction == 5 || direction == 6)  // up
        {
            if (y > -1 + radius)
            {
                y -= speed;
            }
            else
            {
                direction = GetRandomDirection();
            }
        }

        if (direction == 2 || direction == 5 || direction == 7)  // right
        {
            if (x < 1 - radius)
            {
                x += speed;
            }
            else
            {
                direction = GetRandomDirection();
            }
        }

        if (direction == 3 || direction == 7 || direction == 8)  // down
        {
            if (y < 1 - radius)
            {
                y += speed;
            }
            else
            {
                direction = GetRandomDirection();
            }
        }

        if (direction == 4 || direction == 6 || direction == 8)  // left
        {
            if (x > -1 + radius)
            {
                x -= speed;
            }
            else
            {
                direction = GetRandomDirection();
            }
        }
    }

    void DrawCircle()
    {
        glColor3f(red, green, blue);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            float degInRad = i * DEG2RAD;
            glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
        }
        glEnd();
    }

    int GetRandomDirection()
    {
        return (rand() % 8) + 1;
    }
};

vector<Circle> world;
vector<Brick> bricks;

void CreateTriangleFormation(int numRows, float brickWidth)
{
    float startX = -0.5f; // Starting x position for the triangle
    float startY = 0.5f;  // Starting y position for the triangle
    float gap = brickWidth * 1.1f; // Gap between bricks (slightly larger than width)

    // Create the original triangle formation
    for (int row = 0; row < numRows; ++row)
    {
        for (int col = 0; col <= row; ++col)
        {
            float xPos = startX + col * gap;
            float yPos = startY - row * gap;

            bricks.push_back(Brick(DESTRUCTABLE, xPos, yPos, brickWidth, 1.0f, 0.5f, 0.5f)); // All bricks are destructible and red
        }
    }

    // Create the mirrored triangle formation
    for (int row = 0; row < numRows; ++row)
    {
        for (int col = 0; col <= row; ++col)
        {
            float xPos = -startX - col * gap; // Mirroring x position
            float yPos = startY - row * gap;  // Same y position as original

            bricks.push_back(Brick(DESTRUCTABLE, xPos, yPos, brickWidth, 0.5f, 0.5f, 1.0f)); // Mirrored bricks with blue color
        }
    }
}

int main(void) {
    srand(time(NULL));

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(480, 480, "Brick Triangle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Create a triangular formation of bricks
    CreateTriangleFormation(5, 0.1f); // 5 rows of bricks, each 0.1 units wide

    while (!glfwWindowShouldClose(window)) {
        // Setup View
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        processInput(window);

        // Check for collisions and move circles
        vector<Circle> toRemove;
        for (int i = 0; i < world.size(); i++)
        {
            for (Brick& brick : bricks)
            {
                world[i].CheckCollision(&brick);
            }
            world[i].CheckCircleCollision(world, toRemove);
            world[i].MoveOneStep();
            world[i].DrawCircle();
        }

        // Remove collided circles
        for (const auto& circle : toRemove)
        {
            world.erase(remove_if(world.begin(), world.end(), [&](const Circle& c) { return c.x == circle.x && c.y == circle.y; }), world.end());
        }

        // Draw all bricks
        for (Brick& brick : bricks)
        {
            brick.drawBrick();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        // Create a new circle with random direction and white color
        int randomDirection = (rand() % 8) + 1; // Random direction between 1 and 8

        // Adjust the initial position (0, -0.1) to spawn slightly lower from the center
        Circle newCircle(0, -0.3, 0.02, randomDirection, 0.05, 1.0f, 1.0f, 1.0f); // White color
        world.push_back(newCircle);
    }
}
