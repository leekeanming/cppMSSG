#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

// Simple markdown parser
std::string parseMarkdown(const std::string& input) {
  std::istringstream stream(input);
  std::string line, html, paragraph;

  while (std::getline(stream, line)) {
    if (!line.empty() && line[0] == '#') {
      int level = 0;
      while (level < line.size() && line[level] == '#') level++;
      std::string text = line.substr(level + 1); // skip '#' and space
      html += "<h" + std::to_string(level) + ">" + text + "</h" + std::to_string(level) + ">\n";
    } else if (line.empty()) {
      if (!paragraph.empty()) {
        html += "<p>" + paragraph + "</p>\n";
        paragraph.clear();
      }
    } else {
      paragraph += line + " ";
    }
  }
  if (!paragraph.empty()) {
    html += "<p>" + paragraph + "</p>\n";
  }

  // Process links [text](url)
  size_t start = html.find('[');
  while (start != std::string::npos) {
    size_t mid = html.find('](', start);
    size_t end = html.find(')', mid);
    if (mid != std::string::npos && end != std::string::npos) {
      std::string label = html.substr(start + 1, mid - start - 1);
      std::string url = html.substr(mid + 2, end - mid - 2);
      std::string anchor = "<a href=\"" + url + "\">" + label + "</a>";
      html.replace(start, end - start + 1, anchor);
    }
    start = html.find('[', start + 1);
  }

  return html;
}

int main() {
  fs::create_directories("dist");

  fs::path content_dir = "content";
  for (const auto& entry : fs::directory_iterator(content_dir)) {
    if (fs::is_regular_file(entry.status())) {
      std::string file_path = entry.path().string();
      std::ifstream infile(file_path);
      std::string line, markdown;

      while (std::getline(infile, line)) {
        markdown += line + "\n";
      }
      infile.close();

      std::string parsed = parseMarkdown(markdown);
      std::string title = entry.path().stem().string();
      std::string html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">\n"
                          "<title>" + title + "</title></head><body>\n" +
                          parsed + "\n</body></html>";

      std::string output_path = "dist/" + title + ".html";
      std::ofstream outFile(output_path);
      if (outFile.is_open()) {
        outFile << html;
        outFile.close();
        std::cout << "Generated: " << output_path << "\n";
      } else {
        std::cerr << "Cannot write to: " << output_path << "\n";
      }
    }
  }

  return 0;
}
