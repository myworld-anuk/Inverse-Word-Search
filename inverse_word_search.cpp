#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <set>

#define IDX(row, col, width) ((row) * (width) + (col))

std::vector<char> grid;
std::vector<std::string> words;
std::unordered_set<std::string> forbidden_words;
int width, height;
bool all_solutions;
std::vector<std::vector<char>> solutions;
std::vector<int> cell_usage_count;

// Debug function to print the current grid state
void print_grid(std::ostream &out, const std::vector<char> &g)
{
    for (int r = 0; r < height; ++r)
    {
        for (int c = 0; c < width; ++c)
        {
            out << g[IDX(r, c, width)];
        }
        out << std::endl;
    }
    out << std::endl;
}

// Check if a word can be placed at (x, y) in direction (dx, dy)
bool is_valid_position(const std::string &word, int x, int y, int dx, int dy)
{
    int curr_x = x, curr_y = y;

    for (int i = 0; i < word.size(); ++i)
    {
        // Out of bounds check
        if (curr_x < 0 || curr_x >= width || curr_y < 0 || curr_y >= height)
        {
            return false;
        }

        // Check if position is empty (.) or has same character
        if (grid[IDX(curr_y, curr_x, width)] != '.' && grid[IDX(curr_y, curr_x, width)] != word[i])
        {
            return false;
        }

        curr_x += dx;
        curr_y += dy;
    }

    return true;
}

// Place or remove a word at (x, y) in direction (dx, dy)
void place_word(const std::string &word, int x, int y, int dx, int dy, bool place)
{
    int curr_x = x, curr_y = y;

    for (int i = 0; i < word.size(); ++i)
    {
        int idx = IDX(curr_y, curr_x, width);
        if (place)
        {
            grid[idx] = word[i];
            cell_usage_count[idx]++;
        }
        else
        {
            cell_usage_count[idx]--;
            // Only set to '.' if no other word uses this cell
            if (cell_usage_count[idx] == 0)
            {
                grid[idx] = '.';
            }
        }
        curr_x += dx;
        curr_y += dy;
    }
}

// Extract a line of characters from the grid in any direction
std::string extract_line(int start_x, int start_y, int dx, int dy, int length)
{
    std::string line;
    int x = start_x, y = start_y;

    for (int i = 0; i < length; ++i)
    {
        if (x < 0 || x >= width || y < 0 || y >= height)
            break;

        line += grid[IDX(y, x, width)];
        x += dx;
        y += dy;
    }

    return line;
}

// Check if a string contains any forbidden words (forward and backward)
bool contains_forbidden(const std::string &line)
{
    if (line.empty() || line.find('.') != std::string::npos)
        return false;

    for (const std::string &forbidden : forbidden_words)
    {
        if (line.find(forbidden) != std::string::npos)
            return true;

        std::string rev_forbidden = forbidden;
        std::reverse(rev_forbidden.begin(), rev_forbidden.end());
        if (line.find(rev_forbidden) != std::string::npos)
            return true;
    }
    return false;
}

// Overloaded check_forbidden_words for the current grid
bool check_forbidden_words()
{
    // Check rows
    for (int r = 0; r < height; ++r)
    {
        std::string row = extract_line(0, r, 1, 0, width);
        if (row.find('.') == std::string::npos && contains_forbidden(row))
        {
            return true;
        }
    }

    // Check columns
    for (int c = 0; c < width; ++c)
    {
        std::string col = extract_line(c, 0, 0, 1, height);
        if (col.find('.') == std::string::npos && contains_forbidden(col))
        {
            return true;
        }
    }

    // Check diagonals from left to right starting from the first column
    for (int r = 0; r < height; ++r)
    {
        std::string diag = extract_line(0, r, 1, 1, std::min(width, height - r));
        if (diag.length() >= 2 && diag.find('.') == std::string::npos && contains_forbidden(diag))
        {
            return true;
        }
    }

    // Check diagonals from left to right starting from the first row
    for (int c = 1; c < width; ++c)
    {
        std::string diag = extract_line(c, 0, 1, 1, std::min(width - c, height));
        if (diag.length() >= 2 && diag.find('.') == std::string::npos && contains_forbidden(diag))
        {
            return true;
        }
    }

    // Check diagonals from right to left starting from the first column
    for (int r = 0; r < height; ++r)
    {
        std::string diag = extract_line(width - 1, r, -1, 1, std::min(width, height - r));
        if (diag.length() >= 2 && diag.find('.') == std::string::npos && contains_forbidden(diag))
        {
            return true;
        }
    }

    // Check diagonals from right to left starting from the last row
    for (int c = width - 2; c >= 0; --c)
    {
        std::string diag = extract_line(c, 0, -1, 1, std::min(c + 1, height));
        if (diag.length() >= 2 && diag.find('.') == std::string::npos && contains_forbidden(diag))
        {
            return true;
        }
    }

    return false;
}

// Check if the grid is a duplicate of any existing solution
bool is_duplicate_solution(const std::vector<char> &solution)
{
    for (size_t i = 0; i < solutions.size(); ++i)
    {
        if (solutions[i] == solution)
            return true;
    }
    return false;
}

// Check if a specific grid has forbidden words
bool check_grid_for_forbidden_words(const std::vector<char> &test_grid)
{
    // Temporarily swap the grid
    std::vector<char> temp_grid = grid;
    grid = test_grid;

    bool has_forbidden = check_forbidden_words();

    // Restore original grid
    grid = temp_grid;
    return has_forbidden;
}

// Add the current grid and its transformations to the solutions if they are valid
void add_solution_and_equivalents()
{
    if (is_duplicate_solution(grid) || check_grid_for_forbidden_words(grid))
    {
        return;
    }

    // Vectors to store the transformed grids
    std::vector<std::vector<char>> transformed_grids;
    
    // 1. Original grid
    transformed_grids.push_back(grid);
    
    // 2. Vertical flip (rows swapped)
    std::vector<char> vertical_flip(width * height, '.');
    for (int r = 0; r < height; ++r)
    {
        for (int c = 0; c < width; ++c)
        {
            vertical_flip[IDX(height - 1 - r, c, width)] = grid[IDX(r, c, width)];
        }
    }
    transformed_grids.push_back(vertical_flip);
    
    // 3. Reversed words (each row reversed)
    std::vector<char> reversed_words(width * height, '.');
    for (int r = 0; r < height; ++r)
    {
        for (int c = 0; c < width; ++c)
        {
            reversed_words[IDX(r, width - 1 - c, width)] = grid[IDX(r, c, width)];
        }
    }
    transformed_grids.push_back(reversed_words);
    
    // 4. Both rows swapped and words reversed
    std::vector<char> both_transformed(width * height, '.');
    for (int r = 0; r < height; ++r)
    {
        for (int c = 0; c < width; ++c)
        {
            both_transformed[IDX(height - 1 - r, width - 1 - c, width)] = grid[IDX(r, c, width)];
        }
    }
    transformed_grids.push_back(both_transformed);
    
    // Add the valid transformations to the solutions set
    for (size_t i = 0; i < transformed_grids.size(); ++i)
    {
        const std::vector<char> &transformed_grid = transformed_grids[i];
        if (!is_duplicate_solution(transformed_grid) && !check_grid_for_forbidden_words(transformed_grid))
        {
            solutions.push_back(transformed_grid);
        }
    }
}

// Modified fill_empty_cells function to generate all solutions
void fill_empty_cells(int position, std::vector<int> &empty_positions)
{
    // Base case: all positions filled
    if (position >= empty_positions.size())
    {
        if (!check_forbidden_words())
        {
            add_solution_and_equivalents();
        }
        return;
    }
    int current_pos = empty_positions[position];

    // Try each letter of the alphabet for this position
    for (char letter = 'a'; letter <= 'z'; ++letter)
    {
        grid[current_pos] = letter;

        // Quick check if this letter creates a forbidden word in affected lines
        bool valid = true;
        int row = current_pos / width;
        int col = current_pos % width;

        // Check row if all cells in the row are filled
        std::string row_line = extract_line(0, row, 1, 0, width);

        if (row_line.find('.') == std::string::npos && contains_forbidden(row_line))
        {
            valid = false;
        }

        // Check column if all cells in the column are filled
        if (valid)
        {
            std::string col_line = extract_line(col, 0, 0, 1, height);
            if (col_line.find('.') == std::string::npos && contains_forbidden(col_line))
            {
                valid = false;
            }
        }

        // Continue to next position if valid
        if (valid)
        {
            fill_empty_cells(position + 1, empty_positions);
        }
    }

    grid[current_pos] = '.';
}

// Recursive function to find all possible valid word placements
void find_solution(int word_index)
{
    if (word_index == words.size()) {

        // Find all empty positions
        std::vector<int> empty_positions;

        for (int i = 0; i < width * height; ++i) 
        {
            if (grid[i] == '.') 
            {
                empty_positions.push_back(i);
            }
        }

        if (empty_positions.empty()) 
        {
            // If no empty cells, check if solution is valid
            if (!check_forbidden_words()) 
            {
                add_solution_and_equivalents();
            }
        } 
        else 
        {
            // Make a copy of the current grid before filling empty cells
            std::vector<char> grid_copy = grid;

            fill_empty_cells(0, empty_positions);    

            grid = grid_copy;
        }
        return;
    }

    const std::string &word = words[word_index];

    // Define all possible directions
    const int directions[8][2] = {
        {1, 0},   // right
        {0, 1},   // down
        {1, 1},   // diagonal down-right
        {-1, 1},  // diagonal down-left
        {-1, 0},  // left
        {0, -1},  // up
        {-1, -1}, // diagonal up-left
        {1, -1}   // diagonal up-right
    };

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            for (int d = 0; d < 8; ++d)
            {
                int dx = directions[d][0];
                int dy = directions[d][1];

                if (is_valid_position(word, x, y, dx, dy))
                {
                    place_word(word, x, y, dx, dy, true);

                    if (!check_forbidden_words())
                    {
                        // Continue with next word
                        find_solution(word_index + 1);
                    }

                    // Remove the word (backtrack)
                    place_word(word, x, y, dx, dy, false);
                }
            }
        }
    }
}

// Write solutions to output file
void write_solutions(std::ofstream &output_file)
{
    size_t num_solutions_to_write = solutions.size();
    if (!all_solutions && num_solutions_to_write > 0)
    {
        num_solutions_to_write = 1;
    }
    else if (num_solutions_to_write == 0)
    {
        output_file << "No solutions found" << std::endl;
        return;
    }
    else
    {
        output_file << solutions.size() << " solution(s)" << std::endl;
    }

    for (size_t i = 0; i < num_solutions_to_write; ++i)
    {
        output_file << "Board: " << std::endl;

        for (int r = 0; r < height; ++r)
        {
            output_file << " ";
            for (int c = 0; c < width; ++c)
            {
                output_file << solutions[i][IDX(r, c, width)];
            }
            output_file << std::endl;
        }
        output_file << std::endl;
    }
}

// Main solving function
void word_search(std::ofstream &output_file)
{
    grid.assign(width * height, '.');
    cell_usage_count.assign(width * height, 0);

    // Sort words by length (longest first) for better efficiency
    std::sort(words.begin(), words.end(), [](const std::string &a, const std::string &b)
              { return a.size() > b.size(); });

    find_solution(0);

    write_solutions(output_file);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <all_solutions|one_solution>" << std::endl;
        return 1;
    }

    std::ifstream input_file(argv[1]);
    if (!input_file)
    {
        std::cerr << "Failed to open input file: " << argv[1] << std::endl;
        return 1;
    }

    input_file >> width >> height;

    char type;
    std::string word;

    while (input_file >> type >> word)
    {
        if (type == '+')
        {
            words.push_back(word);
        }
        else if (type == '-')
        {
            forbidden_words.insert(word);
        }
    }

    std::ofstream output_file(argv[2]);

    if (!output_file)
    {
        std::cerr << "Failed to open output file: " << argv[2] << std::endl;
        return 1;
    }

    all_solutions = (std::string(argv[3]) == "all_solutions");
    word_search(output_file);

    return 0;
}
