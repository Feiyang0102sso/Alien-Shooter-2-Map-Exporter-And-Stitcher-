#include "map_stitcher.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

void print_usage() {
    std::cerr << "Usage: AS2MapStitcher <folder> [output-folder]\n";
}

std::string remove_surrounding_quotes(std::string value) {
    if (value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.size() - 2);
    }

    return value;
}

std::string read_line(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return remove_surrounding_quotes(value);
}

bool ask_yes_no(const std::string& prompt) {
    const std::string answer = read_line(prompt + " [y/N]: ");
    return answer == "y" || answer == "Y" || answer == "yes" || answer == "YES";
}

void remove_source_tiles(const StitchResult& result) {
    int removed_count = 0;

    for (const auto& path : result.source_tiles) {
        std::error_code error;
        if (fs::remove(path, error) && !error) {
            ++removed_count;
        } else if (error) {
            std::cerr << "Warning: cannot remove " << path
                      << ": " << error.message() << '\n';
        }
    }

    std::cout << "Removed " << removed_count
              << " source tile(s).\n";
}

void run_once() {
    const fs::path input_directory =
        fs::u8path(read_line("Input folder path: "));

    if (!fs::is_directory(input_directory)) {
        throw std::runtime_error("input folder does not exist");
    }

    const fs::path default_output_directory =
        input_directory / "output";
    const std::string output_text = read_line(
        "Output folder (Enter for " +
        default_output_directory.string() + "): ");
    const fs::path output_directory = output_text.empty()
        ? default_output_directory
        : fs::u8path(output_text);

    fs::create_directories(output_directory);
    const StitchResult result = stitch_maps(input_directory, output_directory);

    std::cout << "Completed " << result.map_count << " map(s).\n"
              << "Output folder: " << output_directory << '\n';

    if (ask_yes_no("Remove the BMP fragments used in this run?")) {
        remove_source_tiles(result);
    } else {
        std::cout << "Source fragments were kept.\n";
    }
}

void run_interactive_menu() {
    std::cout << "\n=== AS2 Map Stitcher ===\n";

    while (true) {
        std::cout << "\n1. Stitch a map folder\n"
                     "2. Exit\n";
        const std::string choice = read_line("Choose an option: ");

        if (choice == "2" || choice == "q" || choice == "Q") {
            break;
        }

        if (choice != "1" && !choice.empty()) {
            std::cout << "Please choose 1 or 2.\n";
            continue;
        }

        try {
            run_once();
        } catch (const std::exception& error) {
            std::cerr << "Error: " << error.what() << '\n';
        }

        if (!ask_yes_no("Process another folder?")) {
            break;
        }
    }

    std::cout << "Goodbye.\n";
}

int main(int argc, char** argv) {
    if (argc == 1) {
        run_interactive_menu();
        return 0;
    }

    if (argc != 2 && argc != 3) {
        print_usage();
        return 2;
    }

    try {
        const fs::path input_directory = fs::u8path(argv[1]);
        const fs::path output_directory = argc == 3
            ? fs::u8path(argv[2])
            : input_directory / "output";

        if (!fs::is_directory(input_directory)) {
            throw std::runtime_error("input folder does not exist");
        }

        fs::create_directories(output_directory);
        const StitchResult result = stitch_maps(input_directory, output_directory);

        std::cout << "Finished " << result.map_count << " map(s). Output folder: "
                  << output_directory << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
