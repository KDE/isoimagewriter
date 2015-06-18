#ifndef SUPROGRAM_H
#define SUPROGRAM_H

#include <QString>
#include <QStringList>

// Auxiliary set of classes for handling different kinds of su-applications

// Base class
class SuProgram
{
protected:
    // Name of the su-application
    const QString m_binaryName;
    // Full path to the executable (if present)
    QString m_binaryPath;
    // Additional arguments
    const QStringList m_suArgs;
    // Whether it accepts target command line as separate arguments or single argument
    const bool m_splitArgs;

    // Constructor
    SuProgram(QString binaryName, QStringList suArgs, bool splitArgs);

public:
    // Destructor
    virtual ~SuProgram() {}
    // Check if the program is present in the system; by default searching in PATH is used
    virtual bool isPresent() const;
    // Check whether the su-application is native to the current desktop environment
    virtual bool isNative() const = 0;
    // Restarts the current application with the specified arguments using the GUI su program
    // Returns only if error occurred (execv() is used)
    virtual void restartAsRoot(const QStringList& args);
};


// Derivative classes for specific su-applications

class XdgSu : public SuProgram
{
public:
    XdgSu();
    virtual ~XdgSu() {}
    virtual bool isNative() const;
};

class KdeSu : public SuProgram
{
public:
    KdeSu();
    virtual ~KdeSu() {}
    virtual bool isNative() const;
};

class GkSu : public SuProgram
{
public:
    GkSu();
    virtual ~GkSu() {}
    virtual bool isNative() const;
};


#endif // SUPROGRAM_H
