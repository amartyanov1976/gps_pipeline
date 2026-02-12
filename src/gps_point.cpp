#include "gps_point.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// Конструкторы
GpsPoint::GpsPoint() 
    : latitude_(0.0), longitude_(0.0), speed_(0.0), course_(0.0),
      altitude_(0.0), satellites_(0), hdop_(0.0),
      timestamp_(std::chrono::system_clock::now()), isValid_(false) {}

GpsPoint::GpsPoint(double lat, double lon, double spd, double crs,
                   double alt, int sat, double hdop,
                   const std::chrono::system_clock::time_point& time, bool valid)
    : latitude_(lat), longitude_(lon), speed_(spd), course_(crs),
      altitude_(alt), satellites_(sat), hdop_(hdop),
      timestamp_(time), isValid_(valid) {
    validateLatitude();
    validateLongitude();
    validateCourse();
}

GpsPoint::GpsPoint(const GpsPoint& other)
    : latitude_(other.latitude_), longitude_(other.longitude_),
      speed_(other.speed_), course_(other.course_),
      altitude_(other.altitude_), satellites_(other.satellites_),
      hdop_(other.hdop_), timestamp_(other.timestamp_),
      isValid_(other.isValid_) {}

GpsPoint::GpsPoint(GpsPoint&& other) noexcept
    : latitude_(std::exchange(other.latitude_, 0.0)),
      longitude_(std::exchange(other.longitude_, 0.0)),
      speed_(std::exchange(other.speed_, 0.0)),
      course_(std::exchange(other.course_, 0.0)),
      altitude_(std::exchange(other.altitude_, 0.0)),
      satellites_(std::exchange(other.satellites_, 0)),
      hdop_(std::exchange(other.hdop_, 0.0)),
      timestamp_(std::move(other.timestamp_)),
      isValid_(std::exchange(other.isValid_, false)) {}

// Операторы присваивания
GpsPoint& GpsPoint::operator=(const GpsPoint& other) {
    if (this != &other) {
        latitude_ = other.latitude_;
        longitude_ = other.longitude_;
        speed_ = other.speed_;
        course_ = other.course_;
        altitude_ = other.altitude_;
        satellites_ = other.satellites_;
        hdop_ = other.hdop_;
        timestamp_ = other.timestamp_;
        isValid_ = other.isValid_;
    }
    return *this;
}

GpsPoint& GpsPoint::operator=(GpsPoint&& other) noexcept {
    if (this != &other) {
        latitude_ = std::exchange(other.latitude_, 0.0);
        longitude_ = std::exchange(other.longitude_, 0.0);
        speed_ = std::exchange(other.speed_, 0.0);
        course_ = std::exchange(other.course_, 0.0);
        altitude_ = std::exchange(other.altitude_, 0.0);
        satellites_ = std::exchange(other.satellites_, 0);
        hdop_ = std::exchange(other.hdop_, 0.0);
        timestamp_ = std::move(other.timestamp_);
        isValid_ = std::exchange(other.isValid_, false);
    }
    return *this;
}

// Операторы сравнения
bool GpsPoint::operator==(const GpsPoint& other) const {
    const double EPS = 1e-6;
    return std::abs(latitude_ - other.latitude_) < EPS &&
           std::abs(longitude_ - other.longitude_) < EPS &&
           std::abs(speed_ - other.speed_) < EPS &&
           std::abs(course_ - other.course_) < EPS &&
           std::abs(altitude_ - other.altitude_) < EPS &&
           satellites_ == other.satellites_ &&
           std::abs(hdop_ - other.hdop_) < EPS &&
           isValid_ == other.isValid_;
}

bool GpsPoint::operator!=(const GpsPoint& other) const {
    return !(*this == other);
}

// Геттеры
double GpsPoint::getLatitude() const { return latitude_; }
double GpsPoint::getLongitude() const { return longitude_; }
double GpsPoint::getSpeed() const { return speed_; }
double GpsPoint::getCourse() const { return course_; }
double GpsPoint::getAltitude() const { return altitude_; }
int GpsPoint::getSatellites() const { return satellites_; }
double GpsPoint::getHdop() const { return hdop_; }
std::chrono::system_clock::time_point GpsPoint::getTimestamp() const { return timestamp_; }
bool GpsPoint::isValid() const { return isValid_; }

// Сеттеры
void GpsPoint::setLatitude(double lat) { 
    latitude_ = lat; 
    validateLatitude();
}

void GpsPoint::setLongitude(double lon) { 
    longitude_ = lon; 
    validateLongitude();
}

void GpsPoint::setSpeed(double spd) { speed_ = spd; }
void GpsPoint::setCourse(double crs) { 
    course_ = crs; 
    validateCourse();
}

void GpsPoint::setAltitude(double alt) { altitude_ = alt; }
void GpsPoint::setSatellites(int sat) { satellites_ = sat; }
void GpsPoint::setHdop(double hdop) { hdop_ = hdop; }
void GpsPoint::setTimestamp(const std::chrono::system_clock::time_point& time) { timestamp_ = time; }
void GpsPoint::setValid(bool valid) { isValid_ = valid; }

// Валидация
void GpsPoint::validateLatitude() const {
    if (latitude_ < -90.0 || latitude_ > 90.0) {
        throw std::out_of_range("Latitude must be between -90 and 90 degrees");
    }
}

void GpsPoint::validateLongitude() const {
    if (longitude_ < -180.0 || longitude_ > 180.0) {
        throw std::out_of_range("Longitude must be between -180 and 180 degrees");
    }
}

void GpsPoint::validateCourse() {
    if (course_ < 0.0 || course_ >= 360.0) {
        course_ = std::fmod(course_, 360.0);
        if (course_ < 0) course_ += 360.0;
    }
}

// Методы для работы с точкой
bool GpsPoint::hasPosition() const {
    return std::abs(latitude_) > 0.0001 || std::abs(longitude_) > 0.0001;
}

double GpsPoint::distanceTo(const GpsPoint& other) const {
    const double R = 6371000.0;
    double lat1 = latitude_ * M_PI / 180.0;
    double lat2 = other.latitude_ * M_PI / 180.0;
    double deltaLat = (other.latitude_ - latitude_) * M_PI / 180.0;
    double deltaLon = (other.longitude_ - longitude_) * M_PI / 180.0;
    
    double a = std::sin(deltaLat / 2) * std::sin(deltaLat / 2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(deltaLon / 2) * std::sin(deltaLon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    
    return R * c;
}

std::string GpsPoint::formatTime() const {
    auto time = std::chrono::system_clock::to_time_t(timestamp_);
    std::tm* tm = std::gmtime(&time);
    std::stringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << tm->tm_hour << ":"
       << std::setw(2) << tm->tm_min << ":"
       << std::setw(2) << tm->tm_sec;
    return ss.str();
}

std::string GpsPoint::formatCoordinates() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(5);
    ss << std::abs(latitude_) << "°" << (latitude_ >= 0 ? "N" : "S");
    ss << ", ";
    ss << std::abs(longitude_) << "°" << (longitude_ >= 0 ? "E" : "W");
    return ss.str();
}

std::string GpsPoint::toString() const {
    std::stringstream ss;
    if (isValid_) {
        ss << "GpsPoint{"
           << "lat=" << std::fixed << std::setprecision(5) << latitude_
           << ", lon=" << longitude_
           << ", speed=" << std::setprecision(1) << speed_ << " km/h"
           << ", course=" << course_ << "°"
           << ", alt=" << altitude_ << "m"
           << ", sat=" << satellites_
           << ", hdop=" << hdop_
           << ", time=" << formatTime()
           << "}";
    } else {
        ss << "GpsPoint{INVALID}";
    }
    return ss.str();
}

// Статические методы
GpsPoint GpsPoint::invalid() {
    return GpsPoint();
}

GpsPoint GpsPoint::zero() {
    return GpsPoint(0.0, 0.0, 0.0, 0.0, 0.0, 0, 0.0, 
                    std::chrono::system_clock::now(), true);
}

namespace std {
    size_t hash<GpsPoint>::operator()(const GpsPoint& p) const {
        size_t h1 = std::hash<double>{}(p.getLatitude());
        size_t h2 = std::hash<double>{}(p.getLongitude());
        size_t h3 = std::hash<double>{}(p.getSpeed());
        size_t h4 = std::hash<int>{}(p.getSatellites());
        size_t h5 = std::hash<bool>{}(p.isValid());
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
    }
}