#pragma once

#include <string>

/**
 * Interface for panels that need to respond to tab activation events.
 * Panels implementing this interface will be notified when their tab
 * is activated or deactivated, allowing them to load/unload data on demand.
 */
class IActivityPanel {
public:
    virtual ~IActivityPanel() = default;

    /**
     * Called when the panel's tab becomes active.
     * The panel should load the activity data from the provided file path.
     *
     * @param activityFilePath Full path to the selected activity FIT file.
     *                         Empty string if no activity is selected.
     */
    virtual void OnTabActivated(const std::string& activityFilePath) = 0;

    /**
     * Called when the panel's tab is being deactivated (another tab selected).
     * The panel can use this to clean up resources, stop ongoing operations, etc.
     */
    virtual void OnTabDeactivated() = 0;
};
