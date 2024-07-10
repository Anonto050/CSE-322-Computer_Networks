# CSE-322 | Computer Networks

This repository contains my solutions to the assignments for the course CSE-322 provided by the BUET CSE Department.

## Course Outline
For a detailed overview of the course, refer to the [course outline](https://github.com/Anonto050/CSE-322-Computer_Networks/tree/main/CSE322%20Course%20Outline.pdf).

## Assignments

### Offline-1 | File Server System
**Topics:** `socket programming`, `TCP`

- **[Problem Specification](https://github.com/Anonto050/CSE-322-Computer_Networks/blob/main/Offline1/Problem_Statement.pdf)**
- **[Solution](https://github.com/Anonto050/CSE-322-Computer_Networks/tree/main/Offline1)**

### Offline-2 | Simulating Static and Mobile Dumbbell Topology
**Topics:** `ns3`

- **[Problem Specification](https://github.com/Anonto050/CSE-322-Computer_Networks/blob/main/Offline2/Problem_Statement.pdf)**
- **Static Dumbbell Topology Solution:** [Code](https://github.com/Anonto050/CSE-322-Computer_Networks/tree/main/Offline2/Static_Dumbbell)

    **How to Run:**
    1. Copy `wireless-static-dumbbell.cc` to the `scratch` directory of ns3.
    2. Copy `wireless-static-dumbbell.sh` and `gnuplotter.gp` to the root directory of ns3 (`ns-allinone-[version]/ns-[version]`).
    3. Execute the script:
        ```sh
        bash ./wireless-static-dumbbell.sh
        ```
    4. A `wireless-static-dumbbell-metrics` folder will be generated with the simulation metrics and graphs.

- **Mobile Dumbbell Topology Solution:** [Code](https://github.com/Anonto050/CSE-322-Computer_Networks/tree/main/Offline2/Mobile_Dumbbell)

    **How to Run:**
    1. Copy `wireless-mobile-dumbbell.cc` to the `scratch` directory of ns3.
    2. Copy `wireless-mobile-dumbbell.sh` and `gnuplotter.gp` to the root directory of ns3 (`ns-allinone-[version]/ns-[version]`).
    3. Execute the script:
        ```sh
        bash ./wireless-mobile-dumbbell.sh
        ```
    4. A `wireless-mobile-dumbbell-metrics` folder will be generated with the simulation metrics and graphs.

### Offline-3 | Simulating Congestion Control Algorithms
**Topics:** `ns3`, `TCP Adaptive-Reno`

- **[Problem Specification](https://github.com/Anonto050/CSE-322-Computer_Networks/blob/main/Offline3/Problem_Statement.pdf)**
- **Solution:** [Code](https://github.com/Anonto050/CSE-322-Computer_Networks/tree/main/Offline3)

    **How to Run:**
    1. Copy `tcp-adaptive-reno.cc` and `tcp-adaptive-reno.h` to `src/internet/model` in ns3. Follow the specific instructions provided at the beginning of task-2 in the problem specification.
    2. Create a directory named `congestion_control` in `scratch` and move `congestion-control.cc`, `packet-source.h`, and `packet-source.cc` to this directory.
    3. Copy all `.gp` and `.sh` files to the root directory of ns3.
    4. Execute the script:
        ```sh
        bash ./congestion-control.sh
        ```
    5. Metrics and graphs will be stored in the `congestion-control-metrics` folder.

### Offline-4 | Error Detection and Correction
**Topics:** `Data Link Layer`, `Hamming Code`, `CRC`

- **[Problem Specification](https://github.com/Anonto050/CSE-322-Computer_Networks/blob/main/Offline4/Problem_Statement.docx)**
- **Solution:** [Code](https://github.com/Anonto050/CSE-322-Computer_Networks/blob/main/Offline4/main.cpp)

---

For any queries or issues, feel free to open an issue in this repository or contact me directly.

---

Happy Coding!
