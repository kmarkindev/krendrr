#pragma once

namespace kRendrr
{
    class CheckInitializationMixin
    {
    public:

        bool IsInitialized() const;

    protected:

        /**
         * Together with CheckInitialization allows resources to
         * easily check double initializations or absence of initialization.
         *
         * CheckInitialization is going to throw if we use uninitialized resource.
         */
        void MarkAsInitialized() const;

        void CheckInitialization(bool bShouldBeInitialized = true) const;

    private:

        mutable bool bWasInitialized {false};

    };
}
