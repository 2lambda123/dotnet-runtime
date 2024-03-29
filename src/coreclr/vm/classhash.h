// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

//
// Hash table associated with each module that records for all types defined in that module the mapping
// between type name and token (or TypeHandle).
//

#ifndef __CLASS_HASH_INCLUDED
#define __CLASS_HASH_INCLUDED

#include "dacenumerablehash.h"

// The type of each entry in the hash.
typedef DPTR(struct EEClassHashEntry) PTR_EEClassHashEntry;
class EEClassHashTable;
typedef struct EEClassHashEntry
{
    friend class EEClassHashTable;

#ifdef _DEBUG
    PTR_CUTF8                               DebugKey[2];    // Name of the type
#endif // _DEBUG

    // Accessors for encloser (pointer to hash entry of enclosing type when this entry describes a nested
    // type). You need to use the accessors since the reference is not encoded as a simple pointer anymore.
    PTR_EEClassHashEntry GetEncloser();
    void SetEncloser(EEClassHashEntry *pEncloser) DAC_EMPTY();

    // Bit masks for flags in the data field. <NICE>Ideally we'd abstract this encoding but that's too much
    // code churn right now.</NICE>
    #define EECLASSHASH_TYPEHANDLE_DISCR ((ULONG_PTR)(UINT)0x00000001)
    #define EECLASSHASH_MDEXPORT_DISCR   ((ULONG_PTR)(UINT)0x80000000)
    #define EECLASSHASH_ALREADYSEEN      ((ULONG_PTR)(UINT)0x40000000)

    // Accessors for the data field (TypeHandle or a token with EECLASSHASH_TYPEHANDLE_DISCR set and possibly
    // some of the other flag bits above). The type handle is also encoded as a non-regular pointer, so use
    // this accessor.
    PTR_VOID GetData();
    void SetData(PTR_VOID data) DAC_EMPTY();


private:
    PTR_VOID    m_Data;     // Either the token (if EECLASSHASH_TYPEHANDLE_DISCR), or the type handle encoded
                            // as a relative pointer

    PTR_EEClassHashEntry  m_pEncloser;  // If this entry is a for a nested
                                        // class, this field stores a
                                        // reference to the enclosing type
                                        // (which must be in this same
                                        // hash).
} EEClassHashEntry_t;

// The hash type itself. All common logic is provided by the DacEnumerableHashTable templated base class. See
// DacEnumerableHash.h for details.
typedef DPTR(class EEClassHashTable) PTR_EEClassHashTable;
class EEClassHashTable : public DacEnumerableHashTable<EEClassHashTable, EEClassHashEntry, 4>
{

public:
    // The LookupContext type we export to track GetValue/FindNextNestedClass enumerations is simply a rename
    // of the base classes' hash value enumerator.
    typedef DacEnumerableHashTable<EEClassHashTable, EEClassHashEntry, 4>::LookupContext LookupContext;

    static EEClassHashTable *Create(Module *pModule, DWORD dwNumBuckets, PTR_EEClassHashTable caseSensitiveTable, AllocMemTracker *pamTracker);

    //NOTICE: look at InsertValue() in ClassLoader, that may be the function you want to use. Use this only
    //        when you are sure you want to insert the value in 'this' table. This function does not deal
    //        with case (as often the class loader has to)
    EEClassHashEntry_t *InsertValueUsingPreallocatedEntry(EEClassHashEntry_t *pStorageForNewEntry, LPCUTF8 pszNamespace, LPCUTF8 pszClassName, PTR_VOID Data, EEClassHashEntry_t *pEncloser);
    EEClassHashEntry_t *AllocNewEntry(AllocMemTracker *pamTracker);
    EEClassHashTable   *MakeCaseInsensitiveTable(Module *pModule, AllocMemTracker *pamTracker);

    PTR_EEClassHashEntry FindByNameHandle(const NameHandle* pName);

    BOOL     CompareKeys(PTR_EEClassHashEntry pEntry, LPCUTF8 * pKey2);

    static DWORD    Hash(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, DWORD enclosingHash);

    class ConstructKeyCallback
    {
        public:
            virtual void UseKeys(_In_reads_(2) LPUTF8 *Key) = 0;
    };

    static PTR_VOID CompressClassDef(mdToken cl /* either a TypeDef or ExportedType*/);
    bool UncompressModuleAndClassDef(PTR_VOID Data, Loader::LoadFlag loadFlag,
                                     Module **ppModule, mdTypeDef *pCL,
                                     mdExportedType *pmdFoundExportedType);
    VOID UncompressModuleAndNonExportClassDef(PTR_VOID Data, Module **ppModule,
                                              mdTypeDef *pCL);
    static mdToken UncompressModuleAndClassDef(PTR_VOID Data);

    static DWORD HashEntryToHash(PTR_EEClassHashEntry entry)
    {
        return BaseValuePtrToHash(entry);
    }

private:
#ifndef DACCESS_COMPILE
    EEClassHashTable(Module *pModule, LoaderHeap *pHeap, DWORD cInitialBuckets) :
        DacEnumerableHashTable<EEClassHashTable, EEClassHashEntry, 4>(pModule, pHeap, cInitialBuckets),
        m_pCaseSensitiveTable(NULL) {}
#endif

    VOID ConstructKeyFromData(PTR_EEClassHashEntry pEntry, ConstructKeyCallback * pCallback);

    bool IsCaseInsensitiveTable() { return m_pCaseSensitiveTable != this; }

    static BOOL IsNested(ModuleBase *pModule, mdToken token, mdToken *mdEncloser);
    static BOOL IsNested(const NameHandle* pName, mdToken *mdEncloser);

    PTR_EEClassHashTable m_pCaseSensitiveTable;  // This will be a recursive pointer for the case sensitive table
};

inline DWORD GetHash(PTR_EEClassHashEntry eeclassHashEntry)
{
    // This is only safe, as the only allocator for the EEClassHashEntry type is the hash table
    return EEClassHashTable::HashEntryToHash(eeclassHashEntry);
}

#endif // !__CLASS_HASH_INCLUDED
