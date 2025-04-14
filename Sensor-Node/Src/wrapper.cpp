
// #include "wrapper.h"
// #include "factory.h"

// #include <string>
// #include <stdio.h>

// #include <memory>
// #include <string>
// #include <vector>
// #include <stdexcept>


// struct SensorGroupImpl {
//     std::vector<std::unique_ptr<Sensor>> sensors;
    
//     SensorGroupImpl(const char* type) {
//         std::unique_ptr<SensorFactory> factory;
//         std::string sensorType(type);
        
//         if (sensorType == "Fire") {
//             factory = std::make_unique<FireSensorFactory>();
//             sensors.push_back(factory->createTempSensor());
//             sensors.push_back(factory->createSmokeSensor());
//             sensors.push_back(factory->createGasSensor());
//             sensors.push_back(factory->createFlameSensor());
//         } 
//         else if (sensorType == "Environment") {
//             factory = std::make_unique<EnvironSensorFactory>();
//             sensors.push_back(factory->createHumiditySensor());
//             sensors.push_back(factory->createVOCSensor());
//         }
//         else if (sensorType == "Smart") {
//             factory = std::make_unique<SmartSensorFactory>();
//             sensors.push_back(factory->createAmbientLSensor());
//             sensors.push_back(factory->createThermalISensor());
//         }
//     }

//     size_t count() const { return sensors.size(); }

//     void set_value(int index, float value) {
//         if (index < 0 || index >= static_cast<int>(sensors.size())) {
//             printf("Error: Sensor index %d out of range.\n", index);
//             return;
//         }
//         sensors[index]->setValue(value);
//     }

//     float get_value(int index) const {
//         if (index < 0 || index >= static_cast<int>(sensors.size())) {
//             printf("Error: Sensor index %d out of range.\n", index);
//             return 0.0f; 
//         }
//         return sensors[index]->readValue();
//     }
// };

// // C-compatible interface implementation
// extern "C" {

// SensorGroup create_sensor_group(const char* type) {
//     SensorGroupImpl* group = new SensorGroupImpl(type); 
//     return static_cast<SensorGroup>(group);
// }

// void destroy_sensor_group(SensorGroup group) {
//     if (group) {
//         delete static_cast<SensorGroupImpl*>(group);
//     }
// }

// int get_sensor_count(SensorGroup group) {
//     if (!group) return 0;
//     return static_cast<SensorGroupImpl*>(group)->count();
// }

// void set_sensor_value(SensorGroup group, int index, float value) {
//     if (!group) return;

//     SensorGroupImpl* impl = static_cast<SensorGroupImpl*>(group);
//     if (index < 0 || index >= impl->count()) {
//         printf("Error: Invalid sensor index %d\n", index);
//         return;
//     }
//     impl->set_value(index, value);
// }

// float get_sensor_value(SensorGroup group, int index) {
//     if (!group) return 0.0f;
//     SensorGroupImpl* impl = static_cast<SensorGroupImpl*>(group);
//     if (index < 0 || index >= impl->count()) {
//         printf("Error: Invalid sensor index %d\n", index);
//         return 0.0f;
//     }
//     return impl->get_value(index);
// }

// } // extern "C"
