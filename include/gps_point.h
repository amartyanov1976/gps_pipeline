#pragma once

#include <string>
#include <chrono>
#include <memory>

class GpsPoint {
public:
    // Конструкторы
    GpsPoint();
    GpsPoint(double lat, double lon, double spd, double crs, 
             double alt, int sat, double hdop, 
             const std::chrono::system_clock::time_point& time, bool valid);
    GpsPoint(const GpsPoint& other);
    GpsPoint(GpsPoint&& other) noexcept;
    
    // Деструктор
    ~GpsPoint() = default;
    
    // Операторы присваивания
    GpsPoint& operator=(const GpsPoint& other);
    GpsPoint& operator=(GpsPoint&& other) noexcept;
    
    // Операторы сравнения
    bool operator==(const GpsPoint& other) const;
    bool operator!=(const GpsPoint& other) const;
    
    // Геттеры
    double getLatitude() const;
    double getLongitude() const;
    double getSpeed() const;
    double getCourse() const;
    double getAltitude() const;
    int getSatellites() const;
    double getHdop() const;
    std::chrono::system_clock::time_point getTimestamp() const;
    bool isValid() const;
    bool hasPosition() const;
    
    // Сеттеры
    void setLatitude(double lat);
    void setLongitude(double lon);
    void setSpeed(double spd);
    void setCourse(double crs);
    void setAltitude(double alt);
    void setSatellites(int sat);
    void setHdop(double hdop);
    void setTimestamp(const std::chrono::system_clock::time_point& time);
    void setValid(bool valid);
    
    // Методы для работы с точкой
    double distanceTo(const GpsPoint& other) const;
    std::string formatTime() const;
    std::string formatCoordinates() const;
    std::string toString() const;
    
    // Статические методы
    static GpsPoint invalid();
    static GpsPoint zero();

private:
    double latitude_;
    double longitude_;
    double speed_;
    double course_;
    double altitude_;
    int satellites_;
    double hdop_;
    std::chrono::system_clock::time_point timestamp_;
    bool isValid_;
    
    // Вспомогательные методы
    void validateLatitude() const;
    void validateLongitude() const;
    void validateCourse();
};

namespace std {
    template<>
    struct hash<GpsPoint> {
        size_t operator()(const GpsPoint& p) const;
    };
}