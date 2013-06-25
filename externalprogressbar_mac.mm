////////////////////////////////////////////////////////////////////////////////
// Mac OS X implementation of ExternalProgressBar


#include <QWidget>

#include "externalprogressbar.h"

#include <Cocoa/Cocoa.h>

// Class with platform-specific data
class ExternalProgressBarPrivate
{
public:
    ExternalProgressBarPrivate();
    ~ExternalProgressBarPrivate();

    NSProgressIndicator* progressIndicator;
};

ExternalProgressBarPrivate::ExternalProgressBarPrivate() :
    progressIndicator(nil)
{
}

ExternalProgressBarPrivate::~ExternalProgressBarPrivate()
{
}


ExternalProgressBar::ExternalProgressBar(QWidget* mainWindow) :
    d_ptr(new ExternalProgressBarPrivate()),
    m_MaxValue(0)
{
    Q_UNUSED(mainWindow);

    // Create a new view based on the current application icon with progress bar positioned over it
    // and replace the Dock icon with this view
    NSImageView* iv = [[NSImageView alloc] init];
    [iv setImage: [[NSApplication sharedApplication] applicationIconImage]];
    [[NSApp dockTile] setContentView: iv];

    d_ptr->progressIndicator = [[NSProgressIndicator alloc] initWithFrame: NSMakeRect(0, 0, [NSApp dockTile].size.width, 20)];
    [d_ptr->progressIndicator setHidden: YES];
    [d_ptr->progressIndicator setStyle: NSProgressIndicatorBarStyle];
    [d_ptr->progressIndicator setIndeterminate: NO];
    [d_ptr->progressIndicator setBezeled: YES];
    [iv addSubview: d_ptr->progressIndicator];

    [d_ptr->progressIndicator release];

    [[NSApp dockTile] display];
}

ExternalProgressBar::~ExternalProgressBar()
{
    DestroyProgressBar();
    delete d_ptr;
}

// Initializes the external progress bar and sets its limits
bool ExternalProgressBar::InitProgressBar(quint64 maxSteps)
{
    m_MaxValue = maxSteps;

    [d_ptr->progressIndicator setMinValue: 0];
    [d_ptr->progressIndicator setMaxValue: maxSteps];
    [d_ptr->progressIndicator setDoubleValue: 0];
    [d_ptr->progressIndicator setHidden: NO];
    [[NSApp dockTile] display];

    return true;
}

// Deinitializes the external progress bar and returns into the normal state
bool ExternalProgressBar::DestroyProgressBar()
{
    [d_ptr->progressIndicator setHidden: YES];
    [[NSApp dockTile] display];
    return true;
}

// Updates the current progress bar position
bool ExternalProgressBar::SetProgressValue(quint64 currentSteps)
{
    [d_ptr->progressIndicator setDoubleValue: currentSteps];
    [d_ptr->progressIndicator setHidden: NO];
    [[NSApp dockTile] display];

    return true;
}

// Sets the progress bar to indicate pause
bool ExternalProgressBar::ProgressSetPause()
{
    // Not supported by OS X
    return false;
}

// Sets the progress bar to indicate an error
bool ExternalProgressBar::ProgressSetError()
{
    // Not supported by OS X
    return false;
}

