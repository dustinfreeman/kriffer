// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface*& pInterfaceToRelease)
{
    if (pInterfaceToRelease)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}