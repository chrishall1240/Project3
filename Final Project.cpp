#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <cmath>
#include <random>
#include <ctime>
#include <chrono>
#include <sstream>
#include <SFML/Graphics.hpp>

#define GD_height   100
#define GD_width    295
#define V_height    40
#define V_width     116
#define Button_D    50
#define GR_xStart   1590
#define GR_yStart   50
#define GR_xEnd     GR_xStart + GD_width
#define GR_yEnd     GR_yStart + GD_height
#define FD_xStart   GR_xStart
#define FD_yStart   GR_yEnd + Button_D
#define FD_xEnd     FD_xStart + GD_width
#define FD_yEnd     FD_yStart + GD_height
#define V_xStart    GR_xStart + 10
#define V_yStart    FD_yEnd + Button_D/2
#define V_xEnd      V_xStart + V_width
#define V_yEnd      V_yStart + V_height
#define E_xStart    GR_xEnd - V_width - 10
#define E_yStart    V_yStart
#define E_xEnd      E_xStart + V_width
#define E_yEnd      V_yEnd
#define BFS_xStart  GR_xStart
#define BFS_yStart  V_yEnd + Button_D
#define BFS_xEnd    BFS_xStart + GD_width
#define BFS_yEnd    BFS_yStart + GD_height
#define DFS_xStart  GR_xStart
#define DFS_yStart  BFS_yEnd + Button_D
#define DFS_xEnd    DFS_xStart + GD_width
#define DFS_yEnd    DFS_yStart + GD_height
#define SP_xStart   GR_xStart
#define SP_yStart   DFS_yEnd + Button_D
#define SP_xEnd     SP_xStart + GD_width
#define SP_yEnd     SP_yStart + GD_height

using namespace std;

class Random {
    static mt19937 random;
public:
    static int Int(int min, int max);
    static float Float(float min, float max);
};

mt19937 Random::random(time(0));

int Random::Int(int min, int max)
{
    uniform_int_distribution<int> dist(min, max);
    return dist(random);
}

float Random::Float(float min, float max)
{
    uniform_real_distribution<float> dist(min, max);
    return dist(random);
}

void delay_ms(int ms)
{
    clock_t start_time = clock();
    while (clock() < start_time + ms);
}

enum struct ButtonType { GR, FD, BFS, DFS, SP, V, E };
enum struct ButtonState { Unpressed, Pressed, Hovered };
enum struct Mode { Idle, vertexDraw, edgeSourceSel, edgeDestSel, sourceSel, destSel };

class Line {
public:
    int x1, y1, x2, y2;
    sf::Color color;
    Line(int _x1, int _y1, int _x2, int _y2, sf::Color _color) {
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;
        color = _color;
    }
    void set(int _x1, int _y1, int _x2, int _y2, sf::Color _color) {
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;
        color = _color;
    }
    void draw(sf::RenderWindow& window) {
        sf::Vertex vertices[2];
        vertices[0] = sf::Vertex(sf::Vector2f(x1, y1), color);
        vertices[1] = sf::Vertex(sf::Vector2f(x2, y2), color);
        window.draw(vertices, 2, sf::Lines);

        if (abs(x2 - x1) > abs(y2 - y1)) {
            vertices[0] = sf::Vertex(sf::Vector2f(x1, y1 - 1), color);
            vertices[1] = sf::Vertex(sf::Vector2f(x2, y2 - 1), color);
            window.draw(vertices, 2, sf::Lines);
            vertices[0] = sf::Vertex(sf::Vector2f(x1, y1 + 1), color);
            vertices[1] = sf::Vertex(sf::Vector2f(x2, y2 + 1), color);
            window.draw(vertices, 2, sf::Lines);
        }

        else {
            vertices[0] = sf::Vertex(sf::Vector2f(x1 - 1, y1), color);
            vertices[1] = sf::Vertex(sf::Vector2f(x2 - 1, y2), color);
            window.draw(vertices, 2, sf::Lines);
            vertices[0] = sf::Vertex(sf::Vector2f(x1 + 1, y1), color);
            vertices[1] = sf::Vertex(sf::Vector2f(x2 + 1, y2), color);
            window.draw(vertices, 2, sf::Lines);
        }
    }
};

class Graph {
public:
    vector<tuple<ButtonType, ButtonState, int, int, int, int, int>> buttons;
    sf::Texture GR_Unpressed_Texture, FD_Unpressed_Texture, BFS_Unpressed_Texture, DFS_Unpressed_Texture, SP_Unpressed_Texture, V_Unpressed_Texture, E_Unpressed_Texture;
    sf::Texture GR_Pressed_Texture, FD_Pressed_Texture, BFS_Pressed_Texture, DFS_Pressed_Texture, SP_Pressed_Texture, V_Pressed_Texture, E_Pressed_Texture;
    sf::Texture GR_Hovered_Texture, FD_Hovered_Texture, BFS_Hovered_Texture, DFS_Hovered_Texture, SP_Hovered_Texture, V_Hovered_Texture, E_Hovered_Texture;
    sf::Sprite GR_Unpressed_Sprite, FD_Unpressed_Sprite, BFS_Unpressed_Sprite, DFS_Unpressed_Sprite, SP_Unpressed_Sprite, V_Unpressed_Sprite, E_Unpressed_Sprite;
    sf::Sprite GR_Pressed_Sprite, FD_Pressed_Sprite, BFS_Pressed_Sprite, DFS_Pressed_Sprite, SP_Pressed_Sprite, V_Pressed_Sprite, E_Pressed_Sprite;
    sf::Sprite GR_Hovered_Sprite, FD_Hovered_Sprite, BFS_Hovered_Sprite, DFS_Hovered_Sprite, SP_Hovered_Sprite, V_Hovered_Sprite, E_Hovered_Sprite;
    sf::RectangleShape graphOutline;
    sf::RenderWindow window;
    sf::Text text;
    sf::Font font;
    Mode mode;
    vector<tuple<int, int, sf::Color, int>> vertices;
    vector<vector<tuple<int, sf::Color>>> edges;
    int source, dest;
    float BFSrt, DFSrt, SPrt;
    Graph() {
        init();
    }

    void init() {
        font.loadFromFile("arial.ttf");
        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);
        mode = Mode::Idle;
        source = -1;
        dest = -1;
        BFSrt = -1;
        DFSrt = -1;
        SPrt = -1;

        window.create(sf::VideoMode(1900, 1000), "Graph Traversal Visualizer");
        generateRandom();

        tuple<ButtonType, ButtonState, int, int, int, int, int> button1 = make_tuple(ButtonType::GR, ButtonState::Unpressed, GR_xStart, GR_yStart, GR_xEnd, GR_yEnd, -1);
        buttons.push_back(button1);
        tuple<ButtonType, ButtonState, int, int, int, int, int> button2 = make_tuple(ButtonType::FD, ButtonState::Unpressed, FD_xStart, FD_yStart, FD_xEnd, FD_yEnd, -1);
        buttons.push_back(button2);
        tuple<ButtonType, ButtonState, int, int, int, int, int> button3 = make_tuple(ButtonType::V, ButtonState::Unpressed, V_xStart, V_yStart, V_xEnd, V_yEnd, -1);
        buttons.push_back(button3);
        tuple<ButtonType, ButtonState, int, int, int, int, int> button4 = make_tuple(ButtonType::E, ButtonState::Unpressed, E_xStart, E_yStart, E_xEnd, E_yEnd, -1);
        buttons.push_back(button4);
        tuple<ButtonType, ButtonState, int, int, int, int, int> button5 = make_tuple(ButtonType::BFS, ButtonState::Unpressed, BFS_xStart, BFS_yStart, BFS_xEnd, BFS_yEnd, -1);
        buttons.push_back(button5);
        tuple<ButtonType, ButtonState, int, int, int, int, int> button6 = make_tuple(ButtonType::DFS, ButtonState::Unpressed, DFS_xStart, DFS_yStart, DFS_xEnd, DFS_yEnd, -1);
        buttons.push_back(button6);
        tuple<ButtonType, ButtonState, int, int, int, int, int> button7 = make_tuple(ButtonType::SP, ButtonState::Unpressed, SP_xStart, SP_yStart, SP_xEnd, SP_yEnd, -1);
        buttons.push_back(button7);

        GR_Unpressed_Texture.loadFromFile("images/GR_Unpressed.png");
        FD_Unpressed_Texture.loadFromFile("images/FD_Unpressed.png");
        BFS_Unpressed_Texture.loadFromFile("images/BFS_Unpressed.png");
        DFS_Unpressed_Texture.loadFromFile("images/DFS_Unpressed.png");
        SP_Unpressed_Texture.loadFromFile("images/SP_Unpressed.png");
        V_Unpressed_Texture.loadFromFile("images/V_Unpressed.png");
        E_Unpressed_Texture.loadFromFile("images/E_Unpressed.png");
        GR_Pressed_Texture.loadFromFile("images/GR_Pressed.png");
        FD_Pressed_Texture.loadFromFile("images/FD_Pressed.png");
        BFS_Pressed_Texture.loadFromFile("images/BFS_Pressed.png");
        DFS_Pressed_Texture.loadFromFile("images/DFS_Pressed.png");
        SP_Pressed_Texture.loadFromFile("images/SP_Pressed.png");
        V_Pressed_Texture.loadFromFile("images/V_Pressed.png");
        E_Pressed_Texture.loadFromFile("images/E_Pressed.png");
        GR_Hovered_Texture.loadFromFile("images/GR_Hovered.png");
        FD_Hovered_Texture.loadFromFile("images/FD_Hovered.png");
        BFS_Hovered_Texture.loadFromFile("images/BFS_Hovered.png");
        DFS_Hovered_Texture.loadFromFile("images/DFS_Hovered.png");
        SP_Hovered_Texture.loadFromFile("images/SP_Hovered.png");
        V_Hovered_Texture.loadFromFile("images/V_Hovered.png");
        E_Hovered_Texture.loadFromFile("images/E_Hovered.png");
        GR_Unpressed_Sprite.setTexture(GR_Unpressed_Texture);
        FD_Unpressed_Sprite.setTexture(FD_Unpressed_Texture);
        BFS_Unpressed_Sprite.setTexture(BFS_Unpressed_Texture);
        DFS_Unpressed_Sprite.setTexture(DFS_Unpressed_Texture);
        SP_Unpressed_Sprite.setTexture(SP_Unpressed_Texture);
        V_Unpressed_Sprite.setTexture(V_Unpressed_Texture);
        E_Unpressed_Sprite.setTexture(E_Unpressed_Texture);
        GR_Pressed_Sprite.setTexture(GR_Pressed_Texture);
        FD_Pressed_Sprite.setTexture(FD_Pressed_Texture);
        BFS_Pressed_Sprite.setTexture(BFS_Pressed_Texture);
        DFS_Pressed_Sprite.setTexture(DFS_Pressed_Texture);
        SP_Pressed_Sprite.setTexture(SP_Pressed_Texture);
        V_Pressed_Sprite.setTexture(V_Pressed_Texture);
        E_Pressed_Sprite.setTexture(E_Pressed_Texture);
        GR_Hovered_Sprite.setTexture(GR_Hovered_Texture);
        FD_Hovered_Sprite.setTexture(FD_Hovered_Texture);
        BFS_Hovered_Sprite.setTexture(BFS_Hovered_Texture);
        DFS_Hovered_Sprite.setTexture(DFS_Hovered_Texture);
        SP_Hovered_Sprite.setTexture(SP_Hovered_Texture);
        V_Hovered_Sprite.setTexture(V_Hovered_Texture);
        E_Hovered_Sprite.setTexture(E_Hovered_Texture);
        GR_Unpressed_Sprite.setPosition(GR_xStart, GR_yStart);
        FD_Unpressed_Sprite.setPosition(FD_xStart, FD_yStart);
        BFS_Unpressed_Sprite.setPosition(BFS_xStart, BFS_yStart);
        DFS_Unpressed_Sprite.setPosition(DFS_xStart, DFS_yStart);
        SP_Unpressed_Sprite.setPosition(SP_xStart, SP_yStart);
        V_Unpressed_Sprite.setPosition(V_xStart, V_yStart);
        E_Unpressed_Sprite.setPosition(E_xStart, E_yStart);
        GR_Pressed_Sprite.setPosition(GR_xStart, GR_yStart);
        FD_Pressed_Sprite.setPosition(FD_xStart, FD_yStart);
        BFS_Pressed_Sprite.setPosition(BFS_xStart, BFS_yStart);
        DFS_Pressed_Sprite.setPosition(DFS_xStart, DFS_yStart);
        SP_Pressed_Sprite.setPosition(SP_xStart, SP_yStart);
        V_Pressed_Sprite.setPosition(V_xStart, V_yStart);
        E_Pressed_Sprite.setPosition(E_xStart, E_yStart);
        GR_Hovered_Sprite.setPosition(GR_xStart, GR_yStart);
        FD_Hovered_Sprite.setPosition(FD_xStart, FD_yStart);
        BFS_Hovered_Sprite.setPosition(BFS_xStart, BFS_yStart);
        DFS_Hovered_Sprite.setPosition(DFS_xStart, DFS_yStart);
        SP_Hovered_Sprite.setPosition(SP_xStart, SP_yStart);
        V_Hovered_Sprite.setPosition(V_xStart, V_yStart);
        E_Hovered_Sprite.setPosition(E_xStart, E_yStart);
        graphOutline.setPosition(30, 30);
        graphOutline.setSize(sf::Vector2f(1530, 950));
        graphOutline.setFillColor(sf::Color(120, 150, 190));
        graphOutline.setOutlineThickness(5);
        graphOutline.setOutlineColor(sf::Color::White);
    }

    void generateRandom() {
        vertices.clear();
        edges.clear();
        initVertices();
        initEdges();
    }

    void drawButtons() {
        int delay = 0;
        if (mode == Mode::Idle)
            delay = 1;
        else if (get<1>(buttons[1]) == ButtonState::Pressed)
            delay = 10;
        for (int i = 0; i < buttons.size(); i++) {
            if (get<0>(buttons[i]) == ButtonType::GR) {
                if (get<1>(buttons[i]) == ButtonState::Unpressed)
                    window.draw(GR_Unpressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Pressed)
                    window.draw(GR_Pressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Hovered) {
                    if (get<6>(buttons[i]) >= 250) {
                        GR_Hovered_Sprite.setColor(sf::Color(255, 255, 255, 255));
                        window.draw(GR_Hovered_Sprite);
                    }
                    else {
                        if (get<6>(buttons[i]) == -1)
                            get<6>(buttons[i]) = 0;
                        else {
                            get<6>(buttons[i]) = get<6>(buttons[i]) + 7;
                        }
                        GR_Hovered_Sprite.setColor(sf::Color(255, 255, 255, get<6>(buttons[i])));
                        window.draw(GR_Unpressed_Sprite);
                        window.draw(GR_Hovered_Sprite);
                        delay_ms(delay);
                    }
                }
            }
            else if (get<0>(buttons[i]) == ButtonType::FD) {
                if (get<1>(buttons[i]) == ButtonState::Unpressed)
                    window.draw(FD_Unpressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Pressed)
                    window.draw(FD_Pressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Hovered) {
                    if (get<6>(buttons[i]) >= 250) {
                        FD_Hovered_Sprite.setColor(sf::Color(255, 255, 255, 255));
                        window.draw(FD_Hovered_Sprite);
                    }
                    else {
                        if (get<6>(buttons[i]) == -1)
                            get<6>(buttons[i]) = 0;
                        else {
                            get<6>(buttons[i]) = get<6>(buttons[i]) + 7;
                        }
                        FD_Hovered_Sprite.setColor(sf::Color(255, 255, 255, get<6>(buttons[i])));
                        window.draw(FD_Unpressed_Sprite);
                        window.draw(FD_Hovered_Sprite);
                        delay_ms(delay);
                    }
                }
            }
            else if (get<0>(buttons[i]) == ButtonType::V) {
                if (get<1>(buttons[i]) == ButtonState::Unpressed)
                    window.draw(V_Unpressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Pressed)
                    window.draw(V_Pressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Hovered) {
                    if (get<6>(buttons[i]) >= 250) {
                        V_Hovered_Sprite.setColor(sf::Color(255, 255, 255, 255));
                        window.draw(V_Hovered_Sprite);
                    }
                    else {
                        if (get<6>(buttons[i]) == -1)
                            get<6>(buttons[i]) = 0;
                        else {
                            get<6>(buttons[i]) = get<6>(buttons[i]) + 7;
                        }
                        V_Hovered_Sprite.setColor(sf::Color(255, 255, 255, get<6>(buttons[i])));
                        window.draw(V_Unpressed_Sprite);
                        window.draw(V_Hovered_Sprite);
                        delay_ms(delay);
                    }
                }
            }
            else if (get<0>(buttons[i]) == ButtonType::E) {
                if (get<1>(buttons[i]) == ButtonState::Unpressed)
                    window.draw(E_Unpressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Pressed)
                    window.draw(E_Pressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Hovered) {
                    if (get<6>(buttons[i]) >= 250) {
                        E_Hovered_Sprite.setColor(sf::Color(255, 255, 255, 255));
                        window.draw(E_Hovered_Sprite);
                    }
                    else {
                        if (get<6>(buttons[i]) == -1)
                            get<6>(buttons[i]) = 0;
                        else {
                            get<6>(buttons[i]) = get<6>(buttons[i]) + 7;
                        }
                        E_Hovered_Sprite.setColor(sf::Color(255, 255, 255, get<6>(buttons[i])));
                        window.draw(E_Unpressed_Sprite);
                        window.draw(E_Hovered_Sprite);
                        delay_ms(delay);
                    }
                }
            }
            else if (get<0>(buttons[i]) == ButtonType::BFS) {
                if (get<1>(buttons[i]) == ButtonState::Unpressed)
                    window.draw(BFS_Unpressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Pressed)
                    window.draw(BFS_Pressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Hovered) {
                    if (get<6>(buttons[i]) >= 250) {
                        BFS_Hovered_Sprite.setColor(sf::Color(255, 255, 255, 255));
                        window.draw(BFS_Hovered_Sprite);
                    }
                    else {
                        if (get<6>(buttons[i]) == -1)
                            get<6>(buttons[i]) = 0;
                        else {
                            get<6>(buttons[i]) = get<6>(buttons[i]) + 7;
                        }
                        BFS_Hovered_Sprite.setColor(sf::Color(255, 255, 255, get<6>(buttons[i])));
                        window.draw(BFS_Unpressed_Sprite);
                        window.draw(BFS_Hovered_Sprite);
                        delay_ms(delay);
                    }
                }
            }
            else if (get<0>(buttons[i]) == ButtonType::DFS) {
                if (get<1>(buttons[i]) == ButtonState::Unpressed)
                    window.draw(DFS_Unpressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Pressed)
                    window.draw(DFS_Pressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Hovered) {
                    if (get<6>(buttons[i]) >= 250) {
                        DFS_Hovered_Sprite.setColor(sf::Color(255, 255, 255, 255));
                        window.draw(DFS_Hovered_Sprite);
                    }
                    else {
                        if (get<6>(buttons[i]) == -1)
                            get<6>(buttons[i]) = 0;
                        else {
                            get<6>(buttons[i]) = get<6>(buttons[i]) + 7;
                        }
                        DFS_Hovered_Sprite.setColor(sf::Color(255, 255, 255, get<6>(buttons[i])));
                        window.draw(DFS_Unpressed_Sprite);
                        window.draw(DFS_Hovered_Sprite);
                        delay_ms(delay);
                    }
                }
            }
            else if (get<0>(buttons[i]) == ButtonType::SP) {
                if (get<1>(buttons[i]) == ButtonState::Unpressed)
                    window.draw(SP_Unpressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Pressed)
                    window.draw(SP_Pressed_Sprite);
                else if (get<1>(buttons[i]) == ButtonState::Hovered) {
                    if (get<6>(buttons[i]) >= 250) {
                        SP_Hovered_Sprite.setColor(sf::Color(255, 255, 255, 255));
                        window.draw(SP_Hovered_Sprite);
                    }
                    else {
                        if (get<6>(buttons[i]) == -1)
                            get<6>(buttons[i]) = 0;
                        else {
                            get<6>(buttons[i]) = get<6>(buttons[i]) + 7;
                        }
                        SP_Hovered_Sprite.setColor(sf::Color(255, 255, 255, get<6>(buttons[i])));
                        window.draw(SP_Unpressed_Sprite);
                        window.draw(SP_Hovered_Sprite);
                        delay_ms(delay);
                    }
                }
            }
        }
    }

    void mouseMove(int x, int y) {
        for (int i = 0; i < buttons.size(); i++) {
            if (x > get<2>(buttons[i])) {
                if (x < get<4>(buttons[i])) {
                    if (y > get<3>(buttons[i])) {
                        if (y < get<5>(buttons[i])) {
                            if (get<1>(buttons[i]) == ButtonState::Unpressed || get<1>(buttons[i]) == ButtonState::Hovered) {
                                get<1>(buttons[i]) = ButtonState::Hovered;
                                return;
                            }
                        }
                    }
                }
            }
        }
        for (int i = 0; i < buttons.size(); i++) {
            if (get<1>(buttons[i]) == ButtonState::Hovered) {
                get<1>(buttons[i]) = ButtonState::Unpressed;
                get<6>(buttons[i]) = -1;
            }
        }
    }

    void mouseClick(int x, int y) {

        int button = -1;
        for (int i = 0; i < buttons.size(); i++) {
            if (x > get<2>(buttons[i])) {
                if (x < get<4>(buttons[i])) {
                    if (y > get<3>(buttons[i])) {
                        if (y < get<5>(buttons[i])) {
                            if (get<1>(buttons[i]) == ButtonState::Unpressed || get<1>(buttons[i]) == ButtonState::Hovered) {
                                get<1>(buttons[i]) = ButtonState::Pressed;
                                button = i;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (button != -1) {
            buttonPressed(button);
            return;
        }


        if (mode == Mode::vertexDraw) {
            bool good = false;
            if (x > 50)
                if (x < 1540)
                    if (y > 50)
                        if (y < 950)
                            good = true;
            if (good) {
                for (int j = 0; j < vertices.size(); j++) {
                    if (abs(x - get<0>(vertices[j])) < 50) {
                        if (abs(y - get<1>(vertices[j])) < 50) {
                            good = false;
                            break;
                        }
                    }
                }
            }
            if (good) {
                tuple<int, int, sf::Color, int> temp = make_tuple(x, y, sf::Color::White, 0);
                vertices.push_back(temp);
            }
        }

        else if (mode == Mode::edgeSourceSel || mode == Mode::edgeDestSel || mode == Mode::sourceSel || mode == Mode::destSel) {
            int vertexClicked = -1;
            for (int i = 0; i < vertices.size(); i++) {
                if (x > get<0>(vertices[i]) - 15) {
                    if (x < get<0>(vertices[i]) + 15) {
                        if (y > get<1>(vertices[i]) - 15) {
                            if (y < get<1>(vertices[i]) + 15) {
                                vertexClicked = i;
                                break;
                            }
                        }
                    }
                }
            }
            if (vertexClicked != -1) {
                if (mode == Mode::edgeSourceSel) {
                    source = vertexClicked;
                    mode = Mode::edgeDestSel;
                }
                else if (mode == Mode::edgeDestSel) {
                    mode = Mode::edgeSourceSel;
                    dest = vertexClicked;


                    vector<tuple<int, sf::Color>> temp;
                    tuple<int, sf::Color> t = make_tuple(-1, sf::Color::Black);
                    temp.push_back(t);
                    for (int i = edges.size(); i < vertices.size(); i++)
                        edges.push_back(temp);

                    addEdge(source, dest, true);

                    source = -1;
                    dest = -1;
                }
                else if (mode == Mode::sourceSel) {
                    if (get<1>(buttons[4]) == ButtonState::Pressed) {
                        source = vertexClicked;
                        auto start = chrono::high_resolution_clock::now();
                        BFStime();
                        auto finish = chrono::high_resolution_clock::now();
                        BFS();
                        BFSrt = chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
                        get<1>(buttons[4]) = ButtonState::Unpressed;
                        source = -1;
                        mode = Mode::Idle;
                    }
                    else if (get<1>(buttons[5]) == ButtonState::Pressed) {
                        source = vertexClicked;
                        auto start = chrono::high_resolution_clock::now();
                        DFStime();
                        auto finish = chrono::high_resolution_clock::now();
                        DFS();
                        DFSrt = chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
                        get<1>(buttons[4]) = ButtonState::Unpressed;
                        source = -1;
                        mode = Mode::Idle;
                    }
                    else if (get<1>(buttons[6]) == ButtonState::Pressed) {
                        source = vertexClicked;
                        get<2>(vertices[source]) = sf::Color::Green;
                        mode = Mode::destSel;
                    }
                }
                else if (mode == Mode::destSel) {
                    dest = vertexClicked;
                    get<2>(vertices[dest]) = sf::Color::Green;
                    auto start = chrono::high_resolution_clock::now();
                    SPtime();
                    auto finish = chrono::high_resolution_clock::now();
                    SP();
                    SPrt = chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
                    get<1>(buttons[4]) = ButtonState::Unpressed;
                    dest = -1;
                    source = -1;
                    mode = Mode::Idle;
                }
            }
        }
    }

    void buttonPressed(int button) {
        if (get<0>(buttons[button]) == ButtonType::GR) {
            for (int i = 0; i < buttons.size(); i++)
                if (get<0>(buttons[i]) != ButtonType::GR)
                    get<1>(buttons[i]) = ButtonState::Unpressed;
            draw();
            generateRandom();
            get<1>(buttons[button]) = ButtonState::Unpressed;
            BFSrt = -1;
            DFSrt = -1;
            SPrt = -1;
        }
        else if (get<0>(buttons[button]) == ButtonType::FD) {
            mode = Mode::vertexDraw;
            vertices.clear();
            edges.clear();
            BFSrt = -1;
            DFSrt = -1;
            SPrt = -1;
            get<1>(buttons[2]) = ButtonState::Pressed;
            for (int i = 0; i < buttons.size(); i++)
                if (get<0>(buttons[i]) != ButtonType::FD)
                    if (get<0>(buttons[i]) != ButtonType::V)
                        get<1>(buttons[i]) = ButtonState::Unpressed;
        }
        else if (get<0>(buttons[button]) == ButtonType::V) {
            source = -1;
            dest = -1;
            if (get<1>(buttons[1]) == ButtonState::Pressed) {
                get<1>(buttons[3]) = ButtonState::Unpressed;
                mode = Mode::vertexDraw;
            }
            else
                get<1>(buttons[button]) = ButtonState::Unpressed;
        }
        else if (get<0>(buttons[button]) == ButtonType::E) {
            if (get<1>(buttons[1]) == ButtonState::Pressed) {
                get<1>(buttons[2]) = ButtonState::Unpressed;
                mode = Mode::edgeSourceSel;
            }
            else
                get<1>(buttons[button]) = ButtonState::Unpressed;
        }
        else if (get<0>(buttons[button]) == ButtonType::BFS) {
            for (int i = 0; i < buttons.size(); i++)
                if (get<0>(buttons[i]) != ButtonType::BFS)
                    get<1>(buttons[i]) = ButtonState::Unpressed;
            if (mode == Mode::Idle || mode == Mode::vertexDraw || mode == Mode::edgeSourceSel || mode == Mode::edgeDestSel)
                mode = Mode::sourceSel;
            for (int i = 0; i < vertices.size(); i++)
                get<2>(vertices[i]) = sf::Color::White;
            for (int i = 0; i < edges.size(); i++)
                for (int j = 0; j < edges[i].size(); j++)
                    get<1>(edges[i][j]) = sf::Color::Black;

        }
        else if (get<0>(buttons[button]) == ButtonType::DFS) {
            for (int i = 0; i < buttons.size(); i++)
                if (get<0>(buttons[i]) != ButtonType::DFS)
                    get<1>(buttons[i]) = ButtonState::Unpressed;
            if (mode == Mode::Idle || mode == Mode::vertexDraw || mode == Mode::edgeSourceSel || mode == Mode::edgeDestSel)
                mode = Mode::sourceSel;
            for (int i = 0; i < vertices.size(); i++)
                get<2>(vertices[i]) = sf::Color::White;
            for (int i = 0; i < edges.size(); i++)
                for (int j = 0; j < edges[i].size(); j++)
                    get<1>(edges[i][j]) = sf::Color::Black;
        }
        else if (get<0>(buttons[button]) == ButtonType::SP) {
            for (int i = 0; i < buttons.size(); i++)
                if (get<0>(buttons[i]) != ButtonType::SP)
                    get<1>(buttons[i]) = ButtonState::Unpressed;
            if (mode == Mode::Idle || mode == Mode::vertexDraw || mode == Mode::edgeSourceSel || mode == Mode::edgeDestSel)
                mode = Mode::sourceSel;
            for (int i = 0; i < vertices.size(); i++)
                get<2>(vertices[i]) = sf::Color::White;
            for (int i = 0; i < edges.size(); i++)
                for (int j = 0; j < edges[i].size(); j++)
                    get<1>(edges[i][j]) = sf::Color::Black;
        }
    }

    void initVertices() {
        int x = Random::Int(50, 1540);
        int y = Random::Int(50, 950);
        tuple<int, int, sf::Color, int> temp = make_tuple(0, 0, sf::Color::White, 0);
        bool good = true;
        for (int i = 0; i < 300; i++) {

            while (true) {
                x = Random::Int(50, 1540);
                y = Random::Int(50, 950);
                good = true;
                for (int j = 0; j < vertices.size(); j++) {
                    if (abs(x - get<0>(vertices[j])) < 50) {
                        if (abs(y - get<1>(vertices[j])) < 50) {
                            good = false;
                            break;
                        }
                    }
                }

                if (good) {
                    get<0>(temp) = x;
                    get<1>(temp) = y;
                    vertices.push_back(temp);
                    break;
                }
            }
        }
    }

    bool addEdge(int v1, int v2, bool freeDraw) {
        if (!freeDraw) {
            int x1 = 0;
            int y1 = 0;
            int x2 = 0;
            int y2 = 0;
            int space = 40;
            if (abs(get<0>(vertices[v2]) - get<0>(vertices[v1])) < 40) {
                y1 = get<1>(vertices[v2]);
                y2 = get<1>(vertices[v1]);
                if (get<0>(vertices[v2]) < get<0>(vertices[v1])) {
                    x1 = get<0>(vertices[v2]) - space;
                    x2 = get<0>(vertices[v1]) + space;
                }
                else {
                    x1 = get<0>(vertices[v1]) - space;
                    x2 = get<0>(vertices[v2]) + space;
                }
            }
            else if (abs(get<1>(vertices[v2]) - get<1>(vertices[v1])) < 40) {
                x1 = get<0>(vertices[v2]);
                x2 = get<0>(vertices[v1]);
                if (get<1>(vertices[v2]) < get<1>(vertices[v1])) {
                    y1 = get<1>(vertices[v2]) - space;
                    y2 = get<1>(vertices[v1]) + space;
                }
                else {
                    y1 = get<1>(vertices[v1]) - space;
                    y2 = get<1>(vertices[v2]) + space;
                }
            }
            else {
                x1 = get<0>(vertices[v2]);
                y1 = get<1>(vertices[v2]);
                x2 = get<0>(vertices[v1]);
                y2 = get<1>(vertices[v1]);
            }
            if (x1 > x2) {
                int temp = x1;
                x1 = x2;
                x2 = temp;
            }
            if (y1 > y2) {
                int temp = y1;
                y1 = y2;
                y2 = temp;
            }
            for (int i = 0; i < vertices.size(); i++) {
                if (get<0>(vertices[i]) > x1)
                    if (get<0>(vertices[i]) < x2)
                        if (get<1>(vertices[i]) > y1)
                            if (get<1>(vertices[i]) < y2)
                                if (i != v1)
                                    if (i != v2)
                                        return false;
            }
        }

        if (v1 == v2) {
            return false;
        }
        if (get<3>(vertices[v1]) >= 4) {
            return false;
        }
        if (get<3>(vertices[v2]) >= 4) {
            return false;
        }
        if (get<0>(edges[v1][0]) == -1) {
            get<0>(edges[v1][0]) = v2;
            if (get<0>(edges[v2][0]) == -1)
                get<0>(edges[v2][0]) = v1;
            else {
                tuple<int, sf::Color> temp = make_tuple(v1, sf::Color::Black);
                edges[v2].push_back(temp);
            }

            get<3>(vertices[v1]) = get<3>(vertices[v1]) + 1;
            get<3>(vertices[v2]) = get<3>(vertices[v2]) + 1;
            return true;
        }
        for (int i = 0; i < edges[v1].size(); i++) {
            if (get<0>(edges[v1][i]) == v2)
                return false;
        }
        tuple<int, sf::Color> temp = make_tuple(v2, sf::Color::Black);
        edges[v1].push_back(temp);
        if (get<0>(edges[v2][0]) == -1)
            get<0>(edges[v2][0]) = v1;
        else {
            tuple<int, sf::Color> temp = make_tuple(v1, sf::Color::Black);
            edges[v2].push_back(temp);
        }
        get<3>(vertices[v1]) = get<3>(vertices[v1]) + 1;
        get<3>(vertices[v2]) = get<3>(vertices[v2]) + 1;
        return true;
    }

    void initEdges() {
        vector<tuple<int, sf::Color>> temp;
        tuple<int, sf::Color> t = make_tuple(-1, sf::Color::Black);
        temp.push_back(t);
        for (int i = 0; i < vertices.size(); i++)
            edges.push_back(temp);

        float min = 3000;
        float x1, x2, y1, y2;
        int target = 0;
        int edges = 0;
        vector<int> targets;
        vector<int>::iterator it;
        vector<int> failed;

        for (int i = 0; i < vertices.size(); i++) {
            edges = 4;
            while (get<3>(vertices[i]) < edges) {
                for (int j = 0; j < vertices.size(); j++) {
                    x1 = (float)get<0>(vertices[i]);
                    y1 = (float)get<1>(vertices[i]);
                    x2 = (float)get<0>(vertices[j]);
                    y2 = (float)get<1>(vertices[j]);
                    if (pow(pow(abs(x2 - x1), 2) + pow(abs(y2 - y1), 2), 0.5) < min) {
                        it = find(targets.begin(), targets.end(), j);
                        if (it == targets.end()) {
                            it = find(failed.begin(), failed.end(), j);
                            if (it == failed.end()) {
                                min = pow(pow(abs(x2 - x1), 2) + pow(abs(y2 - y1), 2), 0.5);
                                target = j;
                            }
                        }

                    }
                }
                if (addEdge(i, target, false))
                    targets.push_back(target);
                else
                    failed.push_back(target);
                min = 3000;
                if (failed.size() > 4)
                    break;
            }
            targets.clear();
            failed.clear();
        }
    }

    void drawVertices() {
        sf::CircleShape circle(10.f);
        for (int i = 0; i < vertices.size(); i++) {
            if (i == source || i == dest) {
                if (get<1>(buttons[1]) == ButtonState::Pressed) {
                    circle.setOutlineThickness(2);
                    circle.setOutlineColor(sf::Color::Black);
                }
            }
            else {
                circle.setOutlineThickness(0);
            }
            circle.setPosition(sf::Vector2f(get<0>(vertices[i]) - 10, get<1>(vertices[i]) - 10));
            circle.setFillColor(get<2>(vertices[i]));
            window.draw(circle);
        }
    }

    void drawEdges() {
        Line* line = new Line(0, 0, 0, 0, sf::Color::White);
        for (int i = 0; i < edges.size(); i++) {
            for (int j = 0; j < edges[i].size(); j++) {
                if (get<0>(edges[i][j]) != -1) {
                    line->set(get<0>(vertices[i]), get<1>(vertices[i]), get<0>(vertices[get<0>(edges[i][j])]), get<1>(vertices[get<0>(edges[i][j])]), get<1>(edges[i][j]));
                    line->draw(window);
                }
            }
        }

    }

    void drawTime() {
        if (BFSrt != -1) {
            ostringstream runtime;
            runtime.precision(1);
            runtime << std::fixed << BFSrt;
            text.setString("BFS Runtime: " + runtime.str() +" ns");
            text.setPosition(GR_xStart + 10, SP_yEnd + 50);
            window.draw(text);
        }
        else {
            text.setString("BFS Runtime:");
            text.setPosition(GR_xStart + 10, SP_yEnd + 50);
            window.draw(text);
        }
        
        if (DFSrt != -1) {
            ostringstream runtime;
            runtime.clear();
            runtime.precision(1);
            runtime << std::fixed << DFSrt;
            text.setString("DFS Runtime: " + runtime.str() + " ns");
            text.setPosition(GR_xStart + 10, SP_yEnd + 80);
            window.draw(text);
        }
        else {
            text.setString("DFS Runtime:");
            text.setPosition(GR_xStart + 10, SP_yEnd + 80);
            window.draw(text);
        }

        if (SPrt != -1) {
            ostringstream runtime;
            runtime.clear();
            runtime.precision(1);
            runtime << std::fixed << SPrt;
            text.setString("SP Runtime: " + runtime.str() + " ns");
            text.setPosition(GR_xStart + 10, SP_yEnd + 110);
            window.draw(text);
        }
        else {
            text.setString("SP Runtime:");
            text.setPosition(GR_xStart + 10, SP_yEnd + 110);
            window.draw(text);
        }
    }

    void draw() {
        window.clear(sf::Color(0x31486700));
        window.draw(graphOutline);
        drawEdges();
        drawVertices();
        drawButtons();
        drawTime();
        window.display();
    }

    void BFS() {
        int delay = 0;
        if (vertices.size() < 300)
            delay = 500;
        else
            delay = 5;
        vector<bool> visited;

        for (int i = 0; i < vertices.size(); i++)
            visited.push_back(false);

        queue<tuple<int, int>> q;
        tuple<int, int> temp = make_tuple(source, -1);

        visited[source] = true;
        q.push(temp);

        int currentV = source;

        while (!q.empty())
        {
            currentV = get<0>(q.front());
            if(currentV == source)
                get<2>(vertices[currentV]) = sf::Color::Green;
            else
                get<2>(vertices[currentV]) = sf::Color::Red;

            draw();
            delay_ms(delay);
            q.pop();

            for (int i = 0; i < edges[currentV].size(); i++) {
                if (!visited[get<0>(edges[currentV][i])]) {
                    visited[get<0>(edges[currentV][i])] = true;
                    get<0>(temp) = get<0>(edges[currentV][i]);
                    get<1>(temp) = currentV;
                    q.push(temp);
                }
            }
        }
    }
    void DFS() {
        int delay = 0;
        if (vertices.size() < 300)
            delay = 500;
        else
            delay = 5;

        vector<bool> visited;

        for (int i = 0; i < vertices.size(); i++)
            visited.push_back(false);


        stack<int> s;


        s.push(source);

        int currentV = source;

        while (!s.empty()) {

            currentV = s.top();
            s.pop();

            if (!visited[currentV]) {
                if (currentV == source)
                    get<2>(vertices[currentV]) = sf::Color::Green;
                else
                    get<2>(vertices[currentV]) = sf::Color::Red;               
               draw();
                delay_ms(delay);
                visited[currentV] = true;
            }

            for (int i = 0; i < edges[currentV].size(); i++) {
                if (!visited[get<0>(edges[currentV][i])]) {
                    s.push(get<0>(edges[currentV][i]));
                }
            }
        }
    }
    void SP() {
        int delay = 0;
        if (vertices.size() < 300)
            delay = 500;
        else
            delay = 5;

        vector<bool> visited;

        for (int i = 0; i < vertices.size(); i++)
            visited.push_back(false);


        queue<tuple<int, int>> q;
        tuple<int, int> temp = make_tuple(source, -1);


        visited[source] = true;
        q.push(temp);

        int currentV = source;
        vector<tuple<int, int>> path;

        while (!q.empty())
        {
            currentV = get<0>(q.front());
            path.push_back(q.front());
            if (currentV == source)
                get<2>(vertices[currentV]) = sf::Color::Green;
            else
                get<2>(vertices[currentV]) = sf::Color::Red;
            
            if (currentV != source) {
                for (int i = 0; i < edges[get<1>(q.front())].size(); i++) {
                    if (get<0>(edges[get<1>(q.front())][i]) == currentV) {
                        get<1>(edges[get<1>(q.front())][i]) = sf::Color::Red;
                        break;
                    }
                }
                for (int i = 0; i < edges[currentV].size(); i++) {
                    if (get<0>(edges[currentV][i]) == get<1>(q.front())) {
                        get<1>(edges[currentV][i]) = sf::Color::Red;
                        break;
                    }
                }
            }
            
            draw();
            delay_ms(delay);
            q.pop();
            if (currentV == dest)
                break;

            for (int i = 0; i < edges[currentV].size(); i++) {
                if (!visited[get<0>(edges[currentV][i])]) {
                    visited[get<0>(edges[currentV][i])] = true;
                    get<0>(temp) = get<0>(edges[currentV][i]);
                    get<1>(temp) = currentV;
                    q.push(temp);
                }
            }
        }
        vector<tuple<int, int>> SP;
        temp = path[path.size() - 1];
        SP.push_back(temp);
        while (true) {
            for (int i = 0; i < path.size(); i++) {
                if (get<0>(path[i]) == get<1>(temp)) {
                    temp = path[i];
                    break;
                }
            }
            SP.push_back(temp);
            if (get<0>(temp) == source)
                break;
        }
        for (int i = SP.size() - 2 ; i >= 0; i--) {
            get<2>(vertices[get<0>(SP[i])]) = sf::Color::Green;
            for (int j = 0; j < edges[get<0>(SP[i])].size(); j++) {
                if (get<0>(edges[get<0>(SP[i])][j]) == get<1>(SP[i])) {
                    get<1>(edges[get<0>(SP[i])][j]) = sf::Color::Green;
                }
            }
            for (int j = 0; j < edges[get<1>(SP[i])].size(); j++) {
                if (get<0>(edges[get<1>(SP[i])][j]) == get<0>(SP[i])) {
                    get<1>(edges[get<1>(SP[i])][j]) = sf::Color::Green;
                }
            }
            draw();
            delay_ms(100);
        }
    }
    void BFStime() {

        vector<bool> visited;

        for (int i = 0; i < vertices.size(); i++)
            visited.push_back(false);


        queue<tuple<int, int>> q;
        tuple<int, int> temp = make_tuple(source, -1);


        visited[source] = true;
        q.push(temp);

        int currentV = source;

        while (!q.empty())
        {
            currentV = get<0>(q.front());

            q.pop();

            for (int i = 0; i < edges[currentV].size(); i++) {
                if (!visited[get<0>(edges[currentV][i])]) {
                    visited[get<0>(edges[currentV][i])] = true;
                    get<0>(temp) = get<0>(edges[currentV][i]);
                    get<1>(temp) = currentV;
                    q.push(temp);
                }
            }
        }
    }
    void DFStime() {

        vector<bool> visited;

        for (int i = 0; i < vertices.size(); i++)
            visited.push_back(false);


        stack<int> s;


        s.push(source);

        int currentV = source;

        while (!s.empty()) {
            currentV = s.top();
            s.pop();

            if (!visited[currentV]) {
                visited[currentV] = true;
            }

            for (int i = 0; i < edges[currentV].size(); i++) {
                if (!visited[get<0>(edges[currentV][i])]) {
                    s.push(get<0>(edges[currentV][i]));
                }
            }
        }
    }
    void SPtime() {
        vector<bool> visited;

        for (int i = 0; i < vertices.size(); i++)
            visited.push_back(false);

        queue<tuple<int, int>> q;
        tuple<int, int> temp = make_tuple(source, -1);

        visited[source] = true;
        q.push(temp);

        int currentV = source;
        vector<tuple<int, int>> path;

        while (!q.empty())
        {
            currentV = get<0>(q.front());
            path.push_back(q.front());

            q.pop();
            if (currentV == dest)
                break;

            for (int i = 0; i < edges[currentV].size(); i++) {
                if (!visited[get<0>(edges[currentV][i])]) {
                    visited[get<0>(edges[currentV][i])] = true;
                    get<0>(temp) = get<0>(edges[currentV][i]);
                    get<1>(temp) = currentV;
                    q.push(temp);
                }
            }
        }
        vector<tuple<int, int>> SP;
        temp = path[path.size() - 1];
        SP.push_back(temp);
        while (true) {
            for (int i = 0; i < path.size(); i++) {
                if (get<0>(path[i]) == get<1>(temp)) {
                    temp = path[i];
                    break;
                }
            }
            SP.push_back(temp);
            if (get<0>(temp) == source)
                break;
        }
    }
};

int main()
{
    Graph* graph = new Graph();

    while (graph->window.isOpen())
    {
        sf::Event event;
        while (graph->window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                graph->window.close();
            else if (event.type == sf::Event::MouseMoved) {
                graph->mouseMove(sf::Mouse::getPosition(graph->window).x, sf::Mouse::getPosition(graph->window).y);
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                graph->mouseClick(sf::Mouse::getPosition(graph->window).x, sf::Mouse::getPosition(graph->window).y);
            }
        }
        graph->draw();
    }
}