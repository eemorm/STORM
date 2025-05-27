// STORM Tilemap Editor
// Features: new map creation, map loading, grid editing via keyboard/mouse, save as valid map.json

#include <SFML/Graphics.hpp>
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <vector>

using json = nlohmann::json;

const int TILE_SIZE = 32;

class TileMapEditor {
private:
    std::vector<std::vector<char>> grid;
    int rows, cols;
    int selectedRow = 0, selectedCol = 0;
    sf::Font font;
    sf::Text text;

public:
    TileMapEditor(int r, int c) : rows(r), cols(c) {
        grid.resize(rows, std::vector<char>(cols, ' '));
        font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"); // Adjust path if needed
        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
    }

    int getRows() const { return rows; }
    int getCols() const { return cols; }

    void handleMouseClick(int mouseX, int mouseY) {
        int clickedCol = mouseX / TILE_SIZE;
        int clickedRow = mouseY / TILE_SIZE;

        // Check bounds
        if (clickedRow < 0 || clickedRow >= rows || clickedCol < 0 || clickedCol >= cols)
            return;

        // Select the clicked tile
        selectedRow = clickedRow;
        selectedCol = clickedCol;

        std::cout << "Selected tile (" << selectedRow << ", " << selectedCol << ")\n";
    }

    bool loadFromFile(const std::string& path) {
        std::ifstream inFile(path);
        if (!inFile.is_open()) {
            std::cerr << "Failed to open " << path << std::endl;
            return false;
        }

        json j;
        try {
            inFile >> j;
        } catch (json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            return false;
        }

        if (!j.is_object() && !j.is_array()) {
            std::cerr << "Invalid JSON structure: expecting an object or array\n";
            return false;
        }

        json tiles;

        if (j.is_object()) {
            if (!j.contains("tiles") || !j["tiles"].is_array()) {
                std::cerr << "JSON object does not contain a valid 'tiles' array\n";
                return false;
            }
            tiles = j["tiles"];
        } else {
            tiles = j;
        }

        if (tiles.empty() || !tiles[0].is_array()) {
            std::cerr << "'tiles' must be a 2D array\n";
            return false;
        }

        rows = tiles.size();
        cols = tiles[0].size();
        grid.resize(rows, std::vector<char>(cols));

        for (int y = 0; y < rows; ++y) {
            if (!tiles[y].is_array() || tiles[y].size() != cols) {
                std::cerr << "Row " << y << " has invalid size or is not an array\n";
                return false;
            }
            for (int x = 0; x < cols; ++x) {
                std::string s = tiles[y][x].get<std::string>();
                grid[y][x] = s.empty() ? ' ' : s[0];
            }
        }

        std::cout << "Loaded map.json (" << rows << "x" << cols << ")\n";
        selectedRow = 0;
        selectedCol = 0;

        return true;
    }

    void saveToFile(const std::string& path) {
        json j = json::array();
        for (const auto& row : grid) {
            json line = json::array();
            for (char cell : row) {
                std::string s(1, cell);
                line.push_back(s);
            }
            j.push_back(line);
        }

        std::ofstream outFile(path);
        outFile << j.dump(2);
    }

    void draw(sf::RenderWindow& window) {
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                sf::RectangleShape rect(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                rect.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                rect.setFillColor(sf::Color(50, 50, 50));

                if (x == selectedCol && y == selectedRow) {
                    rect.setFillColor(sf::Color(100, 100, 200)); // Highlight selected tile
                }

                window.draw(rect);

                text.setString(grid[y][x]);

                sf::FloatRect textBounds = text.getLocalBounds();
                float textX = x * TILE_SIZE + (TILE_SIZE - textBounds.width) / 2 - textBounds.left;
                float textY = y * TILE_SIZE + (TILE_SIZE - textBounds.height) / 2 - textBounds.top;
                text.setPosition(textX, textY);

                window.draw(text);
            }
        }
    }

    void handleInput(sf::Keyboard::Key key) {
        if (key == sf::Keyboard::Up)    selectedRow = std::max(0, selectedRow - 1);
        if (key == sf::Keyboard::Down)  selectedRow = std::min(rows - 1, selectedRow + 1);
        if (key == sf::Keyboard::Left)  selectedCol = std::max(0, selectedCol - 1);
        if (key == sf::Keyboard::Right) selectedCol = std::min(cols - 1, selectedCol + 1);
    }

    void handleChar(char c) {
        grid[selectedRow][selectedCol] = c;
    }
};

int main() {
    std::cout << "STORM - Tilemap Editor\n";
    std::cout << "(N)ew map or (L)oad map.json? ";
    char choice;
    std::cin >> choice;

    TileMapEditor editor(1, 1);  // Temporary init

    if (choice == 'L' || choice == 'l') {
        if (!editor.loadFromFile("map.json")) {
            std::cout << "Failed to load map.json. Creating new map.\n";
            int rows, cols;
            std::cout << "Enter number of rows: ";
            while (!(std::cin >> rows) || rows <= 0 || rows > 1000) {
                std::cout << "Please enter a valid positive integer for rows: ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            std::cout << "Enter number of columns: ";
            while (!(std::cin >> cols) || cols <= 0 || cols > 1000) {
                std::cout << "Please enter a valid positive integer for columns: ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            editor = TileMapEditor(rows, cols);
        }
    } else {
        int rows, cols;
        std::cout << "Enter number of rows: ";
        while (!(std::cin >> rows) || rows <= 0 || rows > 1000) {
            std::cout << "Please enter a valid positive integer for rows: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cout << "Enter number of columns: ";
        while (!(std::cin >> cols) || cols <= 0 || cols > 1000) {
            std::cout << "Please enter a valid positive integer for columns: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        editor = TileMapEditor(rows, cols);
    }

    sf::RenderWindow window(sf::VideoMode(editor.getCols() * TILE_SIZE, editor.getRows() * TILE_SIZE), "STORM Editor");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.control && event.key.code == sf::Keyboard::S) {
                    editor.saveToFile("map.json");
                    std::cout << "Saved map.json\n";
                } else {
                    editor.handleInput(event.key.code);
                }
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode >= 32 && event.text.unicode < 128)
                    editor.handleChar(static_cast<char>(event.text.unicode));
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    editor.handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
        }

        window.clear();
        editor.draw(window);
        window.display();
    }

    return 0;
}