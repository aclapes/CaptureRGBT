// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <librealsense2/rs.hpp>     // Include RealSense Cross Platform API
#include <chrono>
#include <thread>
#include <opencv2/opencv.hpp>
#include <queue>
#include <boost/filesystem.hpp>
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time
#include <string>  // string
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include "utils.hpp"

bool debug = true;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace 
{ 
  const size_t ERROR_IN_COMMAND_LINE = 1; 
  const size_t SUCCESS = 0; 
  const size_t ERROR_UNHANDLED_EXCEPTION = 2; 
 
} // namespace

struct Timestamp
{
    std::string id;
    int64_t time;
};

std::vector<std::string> tokenize(std::string s, std::string delimiter)
{
    std::vector<std::string> tokens;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) 
    {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s); // last token

    return tokens;
}

Timestamp process_log_line(std::string line)
{
    std::vector<std::string> tokens = tokenize(line, ",");

    Timestamp ts;
    ts.id = tokens.at(0);
    std::istringstream iss (tokens.at(1));
    iss >> ts.time;
    
    return ts;
}

std::vector<Timestamp> read_log_file(fs::path log_path)
{
    std::ifstream log (log_path.string());
    std::string line;
    std::vector<Timestamp> tokenized_lines;
    if (log.is_open()) {
        while (std::getline(log, line)) {
            Timestamp ts = process_log_line(line);
            tokenized_lines.push_back(ts);
        }
        log.close();
    }

    return tokenized_lines;
}

void time_sync(std::vector<Timestamp> log_a, std::vector<Timestamp> log_b, std::vector<std::pair<Timestamp,Timestamp> > & log_synced, int64_t eps = 50, bool verbose = true)
{
    std::vector<Timestamp> master;
    std::vector<Timestamp> slave;
    if (log_a.size() > log_b.size())
    {
        master = log_a;
        slave  = log_b;
        if (verbose) std::cout << "a is master, b is slave\n";
    } else {
        master = log_b;
        slave  = log_a;
        if (verbose) std::cout << "b is master, a is slave\n";
    }

    int j = 0;

    for (int i = 0; i < master.size(); i++) 
    {
        Timestamp ts_m = master[i];
        std::vector<std::pair<Timestamp, int64_t> > matches;
        while (j < slave.size() && slave[j].time < ts_m.time + eps)
        {
            int64_t dist = abs(ts_m.time - slave[j].time);
            if (dist < eps)
            {
                matches.push_back( std::pair<Timestamp,int64_t>(slave[j], dist) );
            }
            j++;
        }

        if (!matches.empty())
        {
            std::pair<Timestamp, int64_t> m_best = matches[0];
            for (int k = 1; k < matches.size(); k++)
            {
                if (matches[k].second < m_best.second)
                    m_best = matches[k];
            }

            std::pair<Timestamp,Timestamp> synced_pair;
            synced_pair.first  = log_a.size() > log_b.size() ? master[i] : m_best.first;
            synced_pair.second = log_a.size() > log_b.size() ? m_best.first : master[i];
            log_synced.push_back(synced_pair);
        }
        // elif fill_with_previous:
        //     all_matches.append( ((i, ts_m), all_matches[-1][1]) ), ts_m in enumerate(master)
    }

    // for (auto log : log_synced)
    // {
    //     std::cout << log.first.time << "," << log.second.time << '\n';
    // }
    for (int i = 0; i < log_synced.size(); i++)
    {
        std::cout << log_synced[i].first.time << "," << log_synced[i].second.time << '\n';
    }
}


int main(int argc, char * argv[]) try
{
    /* -------------------------------- */
    /*   Command line argument parsing  */
    /* -------------------------------- */
    
    std::string input_dir_str;

    po::options_description desc("Program options");
    desc.add_options()
        ("help,h", "Print help messages")
        ("fps,f", po::value<float>()->default_value(0.f), "Acquisition speed (fps) of realsense (integer number 1~30)")
        ("input-dir", po::value<std::string>(&input_dir_str)->required(), "Input directory containing rs/pt frames and timestamp files");
    
    po::positional_options_description positional_options; 
    positional_options.add("input-dir", 1); 

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(positional_options).run(), vm); // can throw

    // --help option?
    if (vm.count("help"))
    {
        std::cout << "Basic Command Line Parameter App" << std::endl
        << desc << std::endl;
        return SUCCESS;
    }
    
    po::notify(vm); // throws on error, so do after help in case
    
    /* --------------- */
    /*    Main code    */
    /* --------------- */
        
    fs::path input_dir (vm["input-dir"].as<std::string>());

    fs::path log_rs_path (input_dir / fs::path("rs.log"));
    fs::path log_pt_path (input_dir / fs::path("pt.log"));

    std::vector<Timestamp> log_rs = read_log_file(log_rs_path);
    std::vector<Timestamp> log_pt = read_log_file(log_pt_path);

    std::vector<std::pair<Timestamp,Timestamp> > log_synced;
    time_sync(log_rs, log_pt, log_synced);

    fs::path color_path (input_dir / fs::path("rs/color/"));
    fs::path depth_path (input_dir / fs::path("rs/depth/"));
    fs::path therm_path (input_dir / fs::path("pt/thermal/"));

    // cv::namedWindow("Viewer");
    float fps = vm["fps"].as<float>();
    int wait_time = 0;
    if (fps > 0.f) 
        wait_time = (1./fps) * 1000.;

    for (int i = 0; i < log_synced.size() - 1; i++)
    {
        // read timestamps
        Timestamp rs_ts = log_synced[i].first;
        Timestamp pt_ts = log_synced[i].second;

        // read frames
        cv::Mat c = cv::imread(fs::path(color_path / fs::path("c_" + rs_ts.id + ".jpg")).string(), CV_LOAD_IMAGE_UNCHANGED);
        cv::Mat d = cv::imread(fs::path(depth_path / fs::path("d_" + rs_ts.id + ".png")).string(), CV_LOAD_IMAGE_UNCHANGED);
        cv::Mat t = cv::imread(fs::path(therm_path / fs::path("t_" + pt_ts.id + ".png")).string(), CV_LOAD_IMAGE_UNCHANGED);

        // preprocess frames
        d = utils::depth_to_8bit(d, cv::COLORMAP_JET);
        t = utils::thermal_to_8bit(t);
        cv::cvtColor(t, t, cv::COLOR_GRAY2BGR);

        // tile frame images in a mosaic
        std::vector<cv::Mat> frames = {c, d, t};
        cv::Mat tiling;
        utils::tile(frames, 600, 900, 1, 3, tiling);       

        // visualize mosaic
        cv::imshow("Viewer", tiling);
        cv::waitKey(wait_time > 0 ? wait_time : log_synced[i+1].first.time - rs_ts.time);
    }
    // cv::destroyWindow("Viewer");

    return SUCCESS;
}
catch(po::error& e)
{
    std::cerr << "Error parsing arguments: " << e.what() << std::endl;
    return ERROR_IN_COMMAND_LINE;
}
catch (const std::exception & e)
{
    std::cerr << e.what() << std::endl;
    return ERROR_UNHANDLED_EXCEPTION;
}

