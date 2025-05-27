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
        grid.resize(rows, std::vector<char>(cols, '.'));  // default empty tile is '.'
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
        if (!inFile) {
            std::cerr << "Failed to open " << path << "\n";
            return false;
        }

        // 1) Parse JSON:
        json j;
        try {
            inFile >> j;
        } catch (json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << "\n";
            return false;
        }

        // 2) Extract the tiles-array, whichever shape it is:
        json tilesArr;
        if (j.is_object() && j.contains("tiles") && j["tiles"].is_array()) {
            tilesArr = j["tiles"];
        } else if (j.is_array()) {
            tilesArr = j;
        } else {
            std::cerr << "Unexpected JSON format—expected array or { tiles: [...] }\n";
            return false;
        }

        // 3) Build a temporary vector<vector<char>> from that JSON:
        std::vector<std::vector<char>> tempGrid;
        tempGrid.reserve(tilesArr.size());

        size_t maxCols = 0;
        for (const auto& rowJson : tilesArr) {
            if (!rowJson.is_array()) {
                std::cerr << "Each row must be an array\n";
                return false;
            }

            std::vector<char> row;
            row.reserve(rowJson.size());
            for (const auto& cellJson : rowJson) {
                if (cellJson.is_string()) {
                    std::string s = cellJson.get<std::string>();
                    row.push_back(s.empty() ? '.' : s[0]);
                } else {
                    // you could allow numbers, nulls, etc.
                    row.push_back('.');
                }
            }
            maxCols = std::max(maxCols, row.size());
            tempGrid.push_back(std::move(row));
        }

        // 4) **Pad** every row so they all have length = maxCols:
        for (auto& row : tempGrid) {
            while (row.size() < maxCols)
                row.push_back('.');   // Default empty tile is '.'
        }

        // 5) Finally commit into your editor:
        rows = tempGrid.size();
        cols = maxCols;
        grid = std::move(tempGrid);
        selectedRow = selectedCol = 0;

        std::cout << "Loaded map.json (" << rows << "×" << cols << ")\n";
        return true;
    }

    void saveToFile(const std::string& path) {
        // Determine the correct width
        int maxCols = 0;
        for (const auto& row : grid) {
            if (row.size() > maxCols) maxCols = row.size();
        }

        // Pad all rows to match the longest one
        for (auto& row : grid) {
            while (row.size() < maxCols)
                row.push_back('.'); // Default tile fill is '.'
        }

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
        if (!outFile) {
            std::cerr << "Failed to write to file: " << path << "\n";
            return;
        }

        outFile << j.dump(2);
        std::cout << "Saved map.json\n";
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

                text.setString(std::string(1, grid[y][x]));

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
        std::cout << "Writing '" << c << "' to tile (" << selectedRow << ", " << selectedCol << ")\n";
        grid[selectedRow][selectedCol] = c;
    }
};

// Helper function to create an empty map.json file with '.' placeholders
void createEmptyMapFile(int rows, int cols, const std::string& path = "map.json") {
    json j = json::array();

    for (int r = 0; r < rows; ++r) {
        json row = json::array();
        for (int c = 0; c < cols; ++c) {
            row.push_back(".");
        }
        j.push_back(row);
    }

    std::ofstream outFile(path);
    if (!outFile) {
        std::cerr << "Failed to create " << path << "\n";
        return;
    }

    outFile << j.dump(2);
    std::cout << "Created empty map.json (" << rows << "x" << cols << ") with '.' tiles\n";
}

int main() {
    std::cout << "STORM - Tilemap Editor\n";
    std::cout << "(N)ew map or (L)oad map.json? ";
    char choice;
    std::cin >> choice;

    TileMapEditor* editorPtr = nullptr;

    if (choice == 'L' || choice == 'l') {
        auto* tempEditor = new TileMapEditor(1, 1); // temp for loading
        if (!tempEditor->loadFromFile("map.json")) {
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

            createEmptyMapFile(rows, cols, "map.json");
            delete tempEditor;

            auto* newEditor = new TileMapEditor(rows, cols);
            if (!newEditor->loadFromFile("map.json")) {
                std::cerr << "Failed to reload just-created map.json\n";
                delete newEditor;
                return 1;
            }
            editorPtr = newEditor;
        } else {
            editorPtr = tempEditor;
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

        createEmptyMapFile(rows, cols, "map.json");

        auto* tempEditor = new TileMapEditor(1, 1);
        if (!tempEditor->loadFromFile("map.json")) {
            std::cerr << "Failed to reload just-created map.json\n";
            delete tempEditor;
            return 1;
        }
        editorPtr = tempEditor;
    }

    TileMapEditor& editor = *editorPtr;

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
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    editor.handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
        }

        window.clear();
        editor.draw(window);
        window.display();
    }

    delete editorPtr;
    return 0;
}