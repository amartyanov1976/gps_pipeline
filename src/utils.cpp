#include "utils.h"
#include <cmath>

namespace NMEA {

double nmeaToDegrees(const std::string& nmeaCoord, Hemisphere hemisphere) {
    if (nmeaCoord.empty()) {
        return 0.0;
    }
    
    try {
        double value = std::stod(nmeaCoord);
        
        // Формат DDMM.MMMM
        int degrees = static_cast<int>(value / 100);
        double minutes = value - degrees * 100;
        double result = degrees + minutes / 60.0;
        
        // Учитываем полушарие
        if (hemisphere == Hemisphere::SOUTH || hemisphere == Hemisphere::WEST) {
            result = -result;
        }
        
        return result;
    } catch (...) {
        return 0.0;
    }
}

} // namespace NMEA

double GpsPoint::distanceTo(const GpsPoint& other) const {
    // Используем формулу гаверсинусов
    const double R = 6371000.0; // Радиус Земли в метрах
    
    double lat1 = latitude * M_PI / 180.0;
    double lat2 = other.latitude * M_PI / 180.0;
    double deltaLat = (other.latitude - latitude) * M_PI / 180.0;
    double deltaLon = (other.longitude - longitude) * M_PI /