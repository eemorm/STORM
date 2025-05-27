// Custom Includes
#include "nlohmann's JSON Parser/json.hpp"

// SFML
#include "SFML/Audio.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Network.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

// Standard Libraries
#include <iostream>
#include <vector>
#include <fstream>

// Screen Constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

using json = nlohmann::json;

// -- Tile Struct --
struct Tile 
{
    char type;
    sf::RectangleShape shape;
};

// -- Helper: Convert tilemap to JSON --
void exportToJson(const std::vector<std::vector<char>>& map, const std::string& filename) 
{
    json j(map);
    std::ofstream out(filename);
    if (out.is_open()) 
    {
        out << j.dump(4);
        std::cout << "Map saved to: " << filename << "\n";
    } 
    else 
    {
        std::cerr << "Failed to save map to: " << filename << "\n";
    }
}

int main()
{
    int rows = 10;
    int cols = 10;
    int tileSize = 32;

    std::cout << "Enter number of rows: ";
    std::cin >> rows;
    std::cout << "Enter number of columns: ";
    std::cin >> cols;

    sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "STORM", sf::Style::Close); // declare window

    window.setFramerateLimit(60); // set fps limit to avoid too much GPU stress

    std::vector<std::vector<char>> tilemap(rows, std::vector<char>(cols, 'G'));
    std::vector<std::vector<Tile>> visualMap(rows, std::vector<Tile>(cols));

    for (int y = 0; y < rows; ++y) 
    {
        for (int x = 0; x < cols; ++x) 
        {
            visualMap[y][x].type = 'G';
            visualMap[y][x].shape.setSize(sf::Vector2f(tileSize - 1, tileSize - 1));
            visualMap[y][x].shape.setFillColor(sf::Color::Green);
            visualMap[y][x].shape.setPosition(x * tileSize, y * tileSize);
        }
    }

    while (window.isOpen()) // loop when the window is open
    {
        sf::Event event; // declare an event for when an event happens
        
        while (window.pollEvent(event)) // check if window events pressed, i.e. close window
        {
            if (event.type == sf::Event::Closed) // check if window close pressed
            {
                window.close(); // close window
            }      
        }

        window.clear(sf::Color(127, 127, 127)); // turn window to grey
        window.display(); // display output
    }
    
    return 0;
}