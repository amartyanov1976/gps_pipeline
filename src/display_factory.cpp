#include "infra/display_factory.h"
#include "infra/console_display.h"
#include "infra/file_display.h"

std::unique_ptr<IDisplay> DisplayFactory::create(const JsonConfig& config) {
    if (config.getDisplayType() == "file" && !config.getOutputFile().empty()) {
        return std::make_unique<FileDisplay>(
            config.getOutputFile(),
            config.isFileRotation(),
            config.getMaxFileSize()
        );
    }
    return std::make_unique<ConsoleDisplay>();
}
