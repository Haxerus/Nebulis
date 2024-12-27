# Visualizer

Custom real-time point cloud visualizer written in C++ using GLFW. Emphasizes performance while handling large amounts of incoming data (450k+ points per second).

This project stemmed from a lack of existing point cloud visualizers that can handle live rendering of large points clouds (n = 10M+ points) while handling large amounts of incoming point data (450k+ points/s).