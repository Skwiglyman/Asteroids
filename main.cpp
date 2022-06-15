// Steven Weller, ICS2O1, Asteroids Game
// This is a program made in c++ that is meant to replicate the game asteroids
// using ascii characters to display instead of pixels

// importing modules
#include <vector>
#include <iostream>
#include <string>
#include <windows.h>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <time.h>

using namespace std;

// Coord structure that stores x and y Coords. Ex: Coord.x, Coord.y
struct Coord {
    float x;
    float y;
}; 

const int ArrayBorder = 800; // defining width and height of 2d game array
const int GameBorder = 400; // defining the width of the game area

void ClearScreen() // better clar screen function
{	
    COORD cursorPosition;	
    cursorPosition.X = 0;	
    cursorPosition.Y = 0;	
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
    // uses native WIN API commands to clear it
}

// Function to find out if int x is between the low and high range, returns a boolean
bool inRange(int low, int high, int x)
{
    return ((x-high)*(x-low) <= 0);
}

void setCursorPosition(int x, int y) // function that sets cursor console position
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
}

// converts degrees to radians
float degreeToRadian(float angle) {
    return angle*(3.14159265/180);
}

// Line function that uses the equation y = mx + b to generate a line of points between 2 vertices
vector<Coord> line(Coord *p1, Coord *p2) {
    vector<Coord> points;
    if (p1->x > p2->x)
    {
        swap(p1, p2);
    }
    float m = (float(p2->y) - p1->y) / (p2->x+1 - p1->x); // slope
    float b = p1->y; // y intercept
    float dx = p2->x+1 - p1->x; // distance between x1 and x2

    for (float x = 0; x < dx; x+=0.1) {
        float y = m * x + b;
        points.push_back({x + p1->x, y});
    }
    return points;
}

// returns the minimum x coord of the vector
int minWrapX(vector<Coord> coords) {
    int minimum = 0;
    for (int i = 0; i < coords.size(); i++) {if (coords[i].x < minimum) {minimum = coords[i].x;}}
    return -minimum;
}

// returns the maximum x coord of the vector
int maxWrapX(vector<Coord> coords) {
    int maximum = 0;
    for (int i = 0; i < coords.size(); i++) {if (coords[i].x - GameBorder > maximum) {maximum = coords[i].x - GameBorder;}}
    return maximum;
}

// returns the min y coord of the vector
int minWrapY(vector<Coord> coords) {
    int minimum = 0;
    for (int i = 0; i < coords.size(); i++) {if (coords[i].y < minimum) {minimum = coords[i].y;}}
    return -minimum;
}

// returns the max y coord of the vector
int maxWrapY(vector<Coord> coords) {
    int maximum = 0;
    for (int i = 0; i < coords.size(); i++) {if (coords[i].y - GameBorder > maximum) {maximum = coords[i].y - GameBorder;}}
    return maximum;
}


// Polygon Class
class Poly {
public:
    vector<Coord> vertices; // A vector of vertices 
    bool Xout; // boolean to check if the polygon is outside the boundaries on X axis
    bool Yout; // boolean to check if the polygon is outside the boundaries on Y axis

    Poly(vector<Coord> Pvertices) {vertices = Pvertices;} // constructor that lets me set the vertices
    Poly() {} // Constructor that doesn't let me set the vertices

    void wrap() { // Function that moves the polygon to the other side of the screen once it goes outside the screen
        int Xoutside = 0; // counter that hold the number of points outside x boundary
        int Youtside = 0; // counter that hold the number of points outside y boundary

        int Xoutout = 0; // counter that hold the number of points outside x boundary + 100
        int Youtout = 0; // counter that hold the number of points outside y boundary + 100

        for (int i = 0; i < vertices.size(); i++) { 
            if (vertices[i].x > GameBorder+100 or vertices[i].x < -100) {Xoutout += 1;} // counts all points outside of Gameboundary + 100 on X axis
            if (vertices[i].y > GameBorder+100 or vertices[i].y < -100) {Youtout += 1;} // counts all points outside of Gameboundary + 100 on Y axis

            // I count this mostly for the player, because if they get wrapped to the other side of the screen and turn around and go back they won't get wrapped
            // again, this prevents that
            
            if (vertices[i].x > GameBorder or vertices[i].x < 0) {Xoutside += 1;} // counts all points outside of Game boundary on X axis
            if (vertices[i].y > GameBorder or vertices[i].y < 0) {Youtside += 1;} // counts all points outside of Game boundary on Y axis
        }

        if(Xoutside == 0) {Xout = false;} // checks if all points are inside on X axis
        if(Youtside == 0) {Yout = false;} // checks if all points are inside on Y axis

        if(Xoutout == vertices.size()) {Xout = false;} // resets wrapping bool if all points are to far outside, this causes
        if(Youtout == vertices.size()) {Yout = false;} // coordinates to be wrapped again
        
        if (Xoutside == vertices.size() and Xout == false) { // if the polygon has not already wrapped (prevents wrapping continuously)
            int minOffset = minWrapX(vertices); // gets the distance from the min border to the minimum X point
            int maxOffset = maxWrapX(vertices); // gets the distance from the max border to the maximum X point
            for (int i = 0; i < vertices.size(); i++) {
                if (vertices[i].x < 0) {vertices[i].x += GameBorder + minOffset + 10;} // wraps coordinate
                else if (vertices[i].x > GameBorder) {vertices[i].x -= GameBorder + maxOffset + 10;} // wraps coordinate
            }
            Xout = true; // makes sure wrapping will not occur continuously 
        }
        
        // same code here except y coordinate
        if (Youtside == vertices.size() and Yout == false) {
            int minOffset = minWrapY(vertices);
            int maxOffset = maxWrapY(vertices);
            for (int i = 0; i < vertices.size(); i++) {
                if (vertices[i].y < 0) {vertices[i].y += GameBorder + minOffset + 10;}
                else if (vertices[i].y > GameBorder) {vertices[i].y -= GameBorder + maxOffset + 10;}
            }
            Yout = true;
        }
    }

    vector<Coord> allPoints() { // function to generate all points on a polygon
        wrap();
        vector<Coord> lines; // vector of all points in a line
        vector<Coord> points; // vector of all points

        // I generate the line between current point and next point then add it to vector and check if its in game boundary
        for (int current = 0; current < vertices.size(); current++)
        {
            int next = current + 1;
            if (next == vertices.size()) {next = 0;}
            lines = line(&vertices[current], &vertices[next]);
            for (int j = 0; j < lines.size(); j++)
            {
                if (inRange(0, 400, int(lines[j].x)) && inRange(0, 400, int(lines[j].y))) {points.push_back(lines[j]);}
            }
        }
        return points;
    }

    void rotate(float angle) { // rotating points of polygon based on angle
        Coord centroid = {(vertices[0].x+vertices[1].x+vertices[2].x)/3, 
        (vertices[0].y+vertices[1].y+vertices[2].y)/3};

        for (int i = 0; i < vertices.size(); i++) {
            float x = vertices[i].x - centroid.x;
            float y = vertices[i].y - centroid.y;
            float xPrime = x*cos(angle) - y*sin(angle);
            float yPrime = x*sin(angle) + y*cos(angle);

            vertices[i].x = xPrime + centroid.x;
            vertices[i].y = yPrime + centroid.y;

        }
    }
};


class Bullet { // Bullet class
    public:
        float dir; // angle direction
        Coord pos; // x y position
        int speed = 10; // speed
        int life = 40; // how long the bullet 

        Bullet(float Pdir, Coord Ppos) { // Bullet constructor
            dir = Pdir;
            pos = Ppos;
        }

        void move() { // move function
            float xPrime = speed*cos(degreeToRadian(dir)); // getting distance bullet travels on x
            float yPrime = speed*sin(degreeToRadian(dir)); // gettign distance  bulet travels on y

            pos.x += -xPrime; // adding them to current pos
            pos.y += -yPrime;
        }
};

struct Trio { // data type that can hold x y and theta, useful to sort points based on angle
    float x;
    float y;
    int theta;
};


class Asteroid: public Poly { // asteroid class
    public:
        int size = 3; // number of times asteroid can be hit
        Coord spawn; // spawn coordinates
        int minRadius = 30; // minimum radius asteroid can be
        int radiusVariance = 80; // maximum variance in radiance between asteroids
        int radius; // radius
        int dir = rand()%360; // defining angle asteroid will travel in
        Coord speed = {1*cos(degreeToRadian(dir)), 1*sin(degreeToRadian(dir))}; // defining constant speed based on dir
        float rotationRate = (rand()%200)/100-1;

        vector<Trio> verticeInfo; // vector useful for sorting points based on angle to center

        Asteroid(): Poly() {
            generatePoints(); // constructor that auto generates points
        }

        Asteroid(vector<Coord> Pvertices, int Psize): Poly() { // constructor that takes points as input, used when splitting asteroids
            vertices = Pvertices;
            size = Psize;
        }

        void generatePoints() { // function that randomly generates asteroids
            int maxVertices = rand()%11+3; //  I generate the number of vertices
            radius = rand()%radiusVariance+minRadius; // generate the radius
            
            vector<Coord> spawnChoices = {
                {float(rand()%GameBorder), float(-radius)}, {float(rand()%GameBorder), float(radius+GameBorder)},
                {float(-radius), float(rand()%GameBorder)}, {float(radius+GameBorder), float(rand()%GameBorder)},
            };
            
            spawn = spawnChoices[rand()%4]; // I generate the spawning location outside of the game boundary

            for(int i = 0; i < maxVertices; i++) { // randomly generating points
                int theta = rand()%360; // generating angle it will be from center of asteroid
                int dist = rand()%(radius/2) + radius/2; // generating distance from center
                int x = dist*cos(degreeToRadian(theta)); // generating x coord
                int y = dist*sin(degreeToRadian(theta)); // genertating y coord

                verticeInfo.push_back({x+spawn.x, y+spawn.y, theta}); // adding it as a trio
            }
            // I sort the vertices based on angle to center to prevent line crossing
            sort(verticeInfo.begin(), verticeInfo.end(), [](const Trio& lhs, const Trio& rhs) {return lhs.theta < rhs.theta;});
            for(int i = 0; i < verticeInfo.size(); i++) {
                Coord newCoord = {verticeInfo[i].x, verticeInfo[i].y};
                vertices.push_back(newCoord); // I add vertices in order
            }
            // you can find this algorithm in greater detail here: https://observablehq.com/@tarte0/generate-random-simple-polygon
        }

        void move(Coord dist) { // moving asteroid function
            rotate(degreeToRadian(rotationRate));
            for (int i = 0; i < vertices.size(); i++) {
                vertices[i].x += dist.x;
                vertices[i].y += dist.y;
            }
        }
};


class Player: public Poly { // player class
    public:
        float rotation = 90; // defining the rotation of player on start
        float velocity = 0; // defining the velocity of player
        vector<Bullet> bullets; // vector to store fired bullets
        int lastShot = 0; // time since last shot (frames)
        int shootRate = 15; // time required to pass since last shot (frames)
        float rotationRate = 8; // angles rotated per frame
        float velocityCap = 3; // max velocity
        float speedGain = 0.2; // rate of speed gain
        int timeAlive = 0;

        Player(Coord Start): Poly() { // constructor that defines the player starting pos
            vertices = {{Start.x - 6, Start.y + 7}, {Start.x, Start.y - 7}, {Start.x + 6, Start.y + 7}};
            }

        void move(Coord dist) { // moving player function
            for (int i = 0; i < vertices.size(); i++) {
                vertices[i].x += dist.x;
                vertices[i].y += dist.y;
            }
        }

        void rotationInput() { // function that checks if rotation key is press
            if(GetKeyState(VK_LEFT) & 0x8000) { // if left arrow pressed rotate left
                rotate(degreeToRadian(-rotationRate));
                rotation -= rotationRate;
            }

            if(GetKeyState(VK_RIGHT) & 0x8000) { // if left arrow pressed rotate left
                rotate(degreeToRadian(rotationRate));
                rotation += rotationRate;
            }
        }

        void thrustInput() { // function that checks and adds speed based on input
            if(GetKeyState(VK_UP) & 0x8000) { // if up key pressed add foward speed
                velocity += speedGain;
                if (velocity > velocityCap) {velocity = velocityCap;} 
            }
            else if (GetKeyState(VK_DOWN) & 0x8000) { // if up key pressed add backward speed
                velocity -= speedGain;
                if (velocity < -velocityCap) {velocity = -velocityCap;}
            }
        }

        void thrust() { // moving ship based on velocity
            thrustInput();

            float xPrime = -velocity*cos(degreeToRadian(rotation)); // calculates distance ship needs to move on x axis
            float yPrime = -velocity*sin(degreeToRadian(rotation)); // calculates distance ship needs to move on y axis

            move({xPrime, yPrime});
        }

        void shooting() { // shooting function
            if(GetKeyState(VK_SPACE) & 0x8000 and lastShot > shootRate) { // checks if space if pressed
                bullets.push_back(Bullet(rotation, vertices[1])); //  if so adds bullet
                lastShot = 0; // resets time since last shot counter
            }

            for (int i = 0; i < bullets.size(); i++) { // loops through every bullet
                bullets[i].life -= 1; // decrements life counter
                bullets[i].move(); // moves bullet

                if (bullets[i].life <= 0) {bullets.erase(bullets.begin()+i); continue;} // checks if bullet is dead and removes it

                if (bullets[i].pos.x > 400) {bullets[i].pos.x -= 400;} // bullet coordinate wrapping 
                else if (bullets[i].pos.x < 0) {bullets[i].pos.x += 400;}
                if (bullets[i].pos.y > 400) {bullets[i].pos.y -= 400;}
                else if (bullets[i].pos.y < 0) {bullets[i].pos.y += 400;}
            }
        }

        void clock() { // function that manages counters (barely used)
            lastShot += 1; // increments last shot counter
            timeAlive += 1;
        }

        void run() { // run manager function
            rotationInput();
            thrust();
            shooting();
            clock();
        }
};


class gameManager { // class that manages the game
public:
    vector<Asteroid> asteroids; // vector that holds all of the asteroids
    Coord Start = {200, 200}; // start coordinates of player
    Player player = Player(Start); // creating player object
    int asteroidSpawnRate = 90; // defining spawn rate of asteroid

    void spawnAsteroid() { // function that spawns asteroid
        if (rand()%asteroidSpawnRate == 2) {
            asteroids.push_back(Asteroid());
        }
    }

    void moveAsteroids() { // function that moves all asteroids
        for(int i = 0; i<asteroids.size(); i++) {asteroids[i].move(asteroids[i].speed);}
    }

    bool asteroidCollision(Coord p1, Asteroid asteroid) { // function that checks collision between a point and asteroid
        for (int current=0; current<asteroid.vertices.size(); current++) { // loop through all asteroid vertices
            int next = current+1; // counter that holds the index of the next vertice
            if (next == asteroid.vertices.size()) {next = 0;}

            Coord vc = asteroid.vertices[current]; // holds the coordinate of current vertice
            Coord vn = asteroid.vertices[next]; // holds the coordinate of next vertice
            Coord p2 = player.vertices[1]; // defines p2

            // tests if the line between p1 and p2 intersects with the point between vc and vn
            float uA = ((vn.x-vc.x)*(p1.y-vc.y) - (vn.y-vc.y)*(p1.x-vc.x)) / ((vn.y-vc.y)*(p2.x-p1.x) - (vn.x-vc.x)*(p2.y-p1.y));

            float uB = ((p2.x-p1.x)*(p1.y-vc.y) - (p2.y-p1.y)*(p1.x-vc.x)) / ((vn.y-vc.y)*(p2.x-p1.x) - (vn.x-vc.x)*(p2.y-p1.y));

            // if it does it returns true
            if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {return true;}
        }
        // if it does not it returns false
        return false;
    }

    void playerDeath() { // function that tests if player collides with asteroid
        bool death = false;

        for (int j = 0; j<asteroids.size(); j++) {
            for (int i = 0; i<player.vertices.size(); i++) {
                if (asteroidCollision(player.vertices[i], asteroids[j])) { // play collides
                    death = true; // death = True
                }}}

        if (death) { // if death restart game
            asteroids.clear(); // clears asteroids
            player.bullets.clear(); // clears bullets
            player.vertices = {{Start.x - 6, Start.y + 7}, {Start.x, Start.y - 7}, {Start.x + 6, Start.y + 7}}; // moves player
            player.velocity = 0; // resets player velocity
            player.rotation = 90; // resets player rotation
        }
    }

    void splitAsteroids(Asteroid asteroid, int eraseIndex, int size) { // function that splits asteroids
        vector<Coord> split1; // defines split asteroid 1 vertices
        vector<Coord> split2; // defines split asteroid 2 vertices

        int Xsum = 0; // sum of x coords
        int Ysum = 0; // sum of y coords

        // getting sums
        for (int i=0; i<asteroid.vertices.size(); i++) {
            Xsum += asteroid.vertices[i].x;
            Ysum += asteroid.vertices[i].y;
        }

        // finding center of asteroid
        Coord center = {float(Xsum/asteroid.vertices.size()), float(Ysum/asteroid.vertices.size())};

        // making 2 new sets of vertices
        for (int i=0; i<asteroid.vertices.size(); i++) {
            Coord vc = asteroid.vertices[i];
            split1.push_back({(vc.x - center.x)/2 + center.x + 5, (vc.y - center.y)/2 + center.y + 5});
            split2.push_back({(vc.x - center.x)/2 + center.x - 5, (vc.y - center.y)/2 + center.y - 5});
        }
        // erasing original asteroid
        asteroids.erase(asteroids.begin()+eraseIndex);

        // making 2 new asteroids
        asteroids.push_back(Asteroid(split1, size-1));
        asteroids.push_back(Asteroid(split2, size-1));
    }

    void bulletAsteroid() { // function that tests if bullet hits asteroid
        for (int j = 0; j < asteroids.size(); j++) {
            for (int i = 0; i < player.bullets.size(); i++) {
                if (asteroidCollision(player.bullets[i].pos, asteroids[j])) {
                    try { // exception catcher for rare unknown error I couldn't fix
                        player.bullets.erase(player.bullets.begin()+i); // erase bullet
                        // if asteroid is already split too much erase it
                        if (asteroids[j].size <= 1) {asteroids.erase(asteroids.begin()+j); return;} 
                        splitAsteroids(asteroids[j], j, asteroids[j].size); // else split asteroid
                        return;
                    }
                    // else if error return
                    catch(...) {return;}
    }}}}
};


class Screen { // screen class
public:
    gameManager game; // defining game object

    char outputBuffer[ArrayBorder][ArrayBorder]; // defining 2d output array

    void innitOutput() { // function that sets whole array values to ' '
        for (int x = 0; x < ArrayBorder; x++)
        {
            for (int y = 0; y < ArrayBorder; y++)
            {
                outputBuffer[x][y] = ' ';
            }
        }
    }

    void plotPolygons() { // function that gets all of the points in the game and plots them as hashtags to 2d output array
        for (int i = 0; i < game.asteroids.size(); i++) {
            vector<Coord> points = game.asteroids[i].allPoints();
            for (int j = 0; j < points.size(); j++) {
                outputBuffer[int(points[j].x)][int(points[j].y)] = '#';
                }
        }

        vector<Coord> points =  game.player.allPoints();
        for (int j = 0; j < points.size(); j++) {
            outputBuffer[int(points[j].x)][int(points[j].y)] = '#';
            }

        for (int j = 0; j < game.player.bullets.size(); j++) {
            for (int i = -1; i < 2; i++) {
                for (int k = -1; k < 2; k++) {outputBuffer[int(game.player.bullets[j].pos.x + i)][int(game.player.bullets[j].pos.y + k)] = '#';}
            }
        }
    }

    void draw() { // draw function
        innitOutput();
        plotPolygons();
        ClearScreen();
        string output = ""; // defining output string
        for (int y = 0; y < GameBorder; y++) {
            for (int x = 0; x < GameBorder; x++) {
                output += outputBuffer[x][y]; // adding value to output string
            }
            output += "\n"; // adding new line once we reach end of row
        }
        cout << output; // outputting frame
        cout.flush(); // flushing buffer
    }

    void run() { // screen run function
        draw();
        game.player.run();
        game.moveAsteroids();
        game.spawnAsteroid();
        game.bulletAsteroid();
        game.playerDeath();
    }
};

int main() {
    srand(time(NULL)); // setting random seed for rand() functions
    cout.sync_with_stdio(false); // this makes c++ i/o faster
    Screen screen; // defining screen object

    // defining default asteroids
    screen.game.asteroids.push_back(Asteroid());
    screen.game.asteroids.push_back(Asteroid());
    screen.game.asteroids.push_back(Asteroid());
    screen.game.asteroids.push_back(Asteroid());
    screen.game.asteroids.push_back(Asteroid());

    while (true) { // game loop
        try {screen.run();} // exception handling
        catch(...) {}
        Sleep(15); // setting fps
    }
    return 0;
}