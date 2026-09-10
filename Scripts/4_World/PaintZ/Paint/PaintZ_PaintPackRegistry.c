class PaintZ_PaintPackNamespace
{
    string m_Prefix;
    string m_ConfigClass;
    string m_DisplayName;
    string m_Source;
    int m_ApiVersion;
    bool m_Official;
};

class PaintZ_FinishDefinition
{
    string m_Id;
    string m_Prefix;
    string m_OwnerClass;
    string m_DisplayName;
    string m_Type;
    string m_Source;
    bool m_IsPattern;
    ref array<int> m_ScalePercents;
    ref TStringArray m_SurfaceTextures;

    void PaintZ_FinishDefinition()
    {
        m_ScalePercents = new array<int>;
        m_SurfaceTextures = new TStringArray;
    }

    bool AddSurface(int scalePercent, string texture)
    {
        if (scalePercent <= 0 || scalePercent > 1000 || texture == "")
            return false;

        for (int i = 0; i < m_ScalePercents.Count(); i++)
        {
            if (m_ScalePercents.Get(i) == scalePercent)
                return false;
        }

        m_ScalePercents.Insert(scalePercent);
        m_SurfaceTextures.Insert(texture);
        return true;
    }

    bool SupportsScale(int scalePercent)
    {
        for (int i = 0; i < m_ScalePercents.Count(); i++)
        {
            if (m_ScalePercents.Get(i) == scalePercent)
                return true;
        }

        return false;
    }

    string GetSurfaceTexture(int scalePercent)
    {
        for (int i = 0; i < m_ScalePercents.Count(); i++)
        {
            if (m_ScalePercents.Get(i) == scalePercent)
                return m_SurfaceTextures.Get(i);
        }

        if (scalePercent != 100)
        {
            for (int j = 0; j < m_ScalePercents.Count(); j++)
            {
                if (m_ScalePercents.Get(j) == 100)
                    return m_SurfaceTextures.Get(j);
            }
        }

        return "";
    }
};

class PaintZ_PaintPackRegistry
{
    static const int API_VERSION = 1;
    protected static const string PACK_ROOT = "CfgPaintZPacks";
    protected static const string FINISH_ROOT = "CfgPaintZFinishes";
    protected static const string OFFICIAL_PZ_PREFIX = "PZ";
    protected static const string OFFICIAL_PZ_OWNER = "PZ_PaintZOfficial";

    protected static bool s_Initialized;
    protected static ref array<ref PaintZ_PaintPackNamespace> s_Namespaces;
    protected static ref array<ref PaintZ_FinishDefinition> s_Finishes;
    protected static ref TStringArray s_ConflictedPrefixes;

    static void EnsureInitialized()
    {
        if (s_Initialized)
            return;

        s_Initialized = true;
        s_Namespaces = new array<ref PaintZ_PaintPackNamespace>;
        s_Finishes = new array<ref PaintZ_FinishDefinition>;
        s_ConflictedPrefixes = new TStringArray;

        array<ref PaintZ_PaintPackNamespace> ownerCandidates = new array<ref PaintZ_PaintPackNamespace>;
        DiscoverConfigOwners(ownerCandidates);
        ResolveOwners(ownerCandidates);

        array<ref PaintZ_FinishDefinition> finishCandidates = new array<ref PaintZ_FinishDefinition>;
        DiscoverConfigFinishes(finishCandidates);
        ResolveFinishes(finishCandidates);

        PaintZ_PaintLog.Info("paint_pack_registry api=" + API_VERSION + " namespaces=" + s_Namespaces.Count() + " finishes=" + s_Finishes.Count());
    }

    static bool HasFinish(string finishId)
    {
        return GetFinish(finishId) != null;
    }

    static string GetFinishName(string finishId)
    {
        PaintZ_FinishDefinition finish = GetFinish(finishId);
        if (!finish)
            return "";

        return finish.m_DisplayName;
    }

    static bool IsPatternFinish(string finishId)
    {
        PaintZ_FinishDefinition finish = GetFinish(finishId);
        if (!finish)
            return false;

        return finish.m_IsPattern;
    }

    static bool SupportsPatternScale(string finishId, int scalePercent)
    {
        PaintZ_FinishDefinition finish = GetFinish(finishId);
        if (!finish)
            return false;
        if (!finish.m_IsPattern)
            return scalePercent == 100;

        return finish.SupportsScale(scalePercent);
    }

    static string GetSurfaceTexture(string finishId, int scalePercent = 100)
    {
        PaintZ_FinishDefinition finish = GetFinish(finishId);
        if (!finish)
            return "";

        return finish.GetSurfaceTexture(scalePercent);
    }

    static string GetFinishIdByNetworkHash(int finishHash)
    {
        EnsureInitialized();
        if (finishHash == 0)
            return "";

        string result = "";
        for (int i = 0; i < s_Finishes.Count(); i++)
        {
            string finishId = s_Finishes.Get(i).m_Id;
            if (finishId.Hash() != finishHash)
                continue;

            if (result != "")
            {
                PaintZ_PaintLog.Warning("paint_pack_registry network_hash_collision hash=" + finishHash + " ids=" + result + "," + finishId);
                return "";
            }

            result = finishId;
        }

        return result;
    }

    static bool IsRegisteredSurfaceTexture(string texture)
    {
        EnsureInitialized();
        if (texture == "")
            return false;

        string normalized = texture;
        normalized.ToLower();
        normalized.Replace("/", "\\");

        for (int i = 0; i < s_Finishes.Count(); i++)
        {
            PaintZ_FinishDefinition finish = s_Finishes.Get(i);
            for (int j = 0; j < finish.m_SurfaceTextures.Count(); j++)
            {
                string candidate = finish.m_SurfaceTextures.Get(j);
                candidate.ToLower();
                candidate.Replace("/", "\\");
                if (candidate == normalized)
                    return true;
            }
        }

        return false;
    }

    static bool IsNamespaceValid(string prefix)
    {
        EnsureInitialized();
        prefix.ToUpper();
        return GetNamespace(prefix) != null;
    }

    protected static PaintZ_FinishDefinition GetFinish(string finishId)
    {
        EnsureInitialized();
        finishId.ToUpper();

        for (int i = 0; i < s_Finishes.Count(); i++)
        {
            PaintZ_FinishDefinition finish = s_Finishes.Get(i);
            if (finish.m_Id == finishId)
                return finish;
        }

        return null;
    }

    protected static PaintZ_PaintPackNamespace GetNamespace(string prefix)
    {
        for (int i = 0; i < s_Namespaces.Count(); i++)
        {
            PaintZ_PaintPackNamespace owner = s_Namespaces.Get(i);
            if (owner.m_Prefix == prefix)
                return owner;
        }

        return null;
    }

    protected static void DiscoverConfigOwners(array<ref PaintZ_PaintPackNamespace> candidates)
    {
        int count = GetGame().ConfigGetChildrenCount(PACK_ROOT);
        for (int i = 0; i < count; i++)
        {
            string child;
            GetGame().ConfigGetChildName(PACK_ROOT, i, child);
            if (child == "")
                continue;

            string path = PACK_ROOT + " " + child;
            PaintZ_PaintPackNamespace owner = new PaintZ_PaintPackNamespace();
            owner.m_ConfigClass = child;
            owner.m_Source = path;
            owner.m_ApiVersion = GetGame().ConfigGetInt(path + " apiVersion");
            owner.m_Official = GetGame().ConfigGetInt(path + " official") == 1;
            GetGame().ConfigGetText(path + " prefix", owner.m_Prefix);
            GetGame().ConfigGetText(path + " displayName", owner.m_DisplayName);
            owner.m_Prefix.ToUpper();

            if (owner.m_ApiVersion != API_VERSION)
            {
                PaintZ_PaintLog.Warning("paint_pack_registry owner_rejected source=" + path + " reason=unsupported_api version=" + owner.m_ApiVersion);
                continue;
            }

            if (!ValidatePrefix(owner.m_Prefix))
            {
                PaintZ_PaintLog.Warning("paint_pack_registry owner_rejected source=" + path + " reason=invalid_prefix prefix=" + owner.m_Prefix);
                continue;
            }

            if (IsReservedPrefix(owner.m_Prefix) && !IsAssignedOfficialOwner(owner))
            {
                PaintZ_PaintLog.Warning("paint_pack_registry owner_rejected source=" + path + " reason=reserved_owner_not_assigned prefix=" + owner.m_Prefix + " owner=" + owner.m_ConfigClass);
                continue;
            }

            candidates.Insert(owner);
        }
    }

    protected static void ResolveOwners(array<ref PaintZ_PaintPackNamespace> candidates)
    {
        TStringArray processed = new TStringArray;
        for (int i = 0; i < candidates.Count(); i++)
        {
            PaintZ_PaintPackNamespace candidate = candidates.Get(i);
            if (ContainsString(processed, candidate.m_Prefix))
                continue;

            processed.Insert(candidate.m_Prefix);
            int matches = 0;
            string sources = "";
            for (int j = 0; j < candidates.Count(); j++)
            {
                PaintZ_PaintPackNamespace other = candidates.Get(j);
                if (other.m_Prefix != candidate.m_Prefix)
                    continue;

                matches++;
                if (sources != "")
                    sources += ", ";
                sources += other.m_Source;
            }

            if (matches != 1)
            {
                s_ConflictedPrefixes.Insert(candidate.m_Prefix);
                PaintZ_PaintLog.Warning("paint_pack_registry namespace_conflict prefix=" + candidate.m_Prefix + " owners=" + sources + " action=namespace_disabled");
                continue;
            }

            s_Namespaces.Insert(candidate);
            PaintZ_PaintLog.Info("paint_pack_registry namespace_registered prefix=" + candidate.m_Prefix + " owner=" + candidate.m_ConfigClass + " source=" + candidate.m_Source);
        }
    }

    protected static void DiscoverConfigFinishes(array<ref PaintZ_FinishDefinition> candidates)
    {
        int count = GetGame().ConfigGetChildrenCount(FINISH_ROOT);
        for (int i = 0; i < count; i++)
        {
            string child;
            GetGame().ConfigGetChildName(FINISH_ROOT, i, child);
            if (child == "")
                continue;

            string path = FINISH_ROOT + " " + child;
            PaintZ_FinishDefinition finish = ReadConfigFinish(path);
            if (finish)
                candidates.Insert(finish);
        }
    }

    protected static PaintZ_FinishDefinition ReadConfigFinish(string path)
    {
        PaintZ_FinishDefinition finish = new PaintZ_FinishDefinition();
        finish.m_Source = path;
        GetGame().ConfigGetText(path + " id", finish.m_Id);
        GetGame().ConfigGetText(path + " owner", finish.m_OwnerClass);
        GetGame().ConfigGetText(path + " displayName", finish.m_DisplayName);
        GetGame().ConfigGetText(path + " type", finish.m_Type);
        finish.m_IsPattern = GetGame().ConfigGetInt(path + " isPattern") == 1;
        finish.m_Id.ToUpper();
        finish.m_Type.ToLower();

        string prefix;
        string typeCode;
        string suffix;
        if (!ParseFinishId(finish.m_Id, prefix, typeCode, suffix))
        {
            PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=invalid_id id=" + finish.m_Id);
            return null;
        }

        finish.m_Prefix = prefix;
        if (!IsNamespaceUsable(prefix))
        {
            PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=namespace_unavailable prefix=" + prefix);
            return null;
        }

        PaintZ_PaintPackNamespace owner = GetNamespace(prefix);
        if (!owner || finish.m_OwnerClass == "" || finish.m_OwnerClass != owner.m_ConfigClass)
        {
            string expectedOwner = "<none>";
            if (owner)
                expectedOwner = owner.m_ConfigClass;
            PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=owner_mismatch prefix=" + prefix + " owner=" + finish.m_OwnerClass + " expected=" + expectedOwner);
            return null;
        }

        if (finish.m_DisplayName == "" || !TypeMatchesCode(finish.m_Type, typeCode))
        {
            PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=invalid_metadata id=" + finish.m_Id);
            return null;
        }

        string surfacesPath = path + " Surfaces";
        int surfaceCount = GetGame().ConfigGetChildrenCount(surfacesPath);
        for (int surfaceIndex = 0; surfaceIndex < surfaceCount; surfaceIndex++)
        {
            string surfaceChild;
            GetGame().ConfigGetChildName(surfacesPath, surfaceIndex, surfaceChild);
            if (surfaceChild == "")
                continue;

            string surfacePath = surfacesPath + " " + surfaceChild;
            int scalePercent = GetGame().ConfigGetInt(surfacePath + " scalePercent");
            string texture;
            GetGame().ConfigGetText(surfacePath + " texture", texture);
            if (!finish.AddSurface(scalePercent, texture))
            {
                PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=invalid_surface child=" + surfaceChild);
                return null;
            }
        }

        if (!finish.SupportsScale(100))
        {
            PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=missing_scale_100 id=" + finish.m_Id);
            return null;
        }

        if (!finish.m_IsPattern && finish.m_ScalePercents.Count() != 1)
        {
            PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=non_pattern_has_scaled_surfaces id=" + finish.m_Id);
            return null;
        }

        if (finish.m_Type == "basic")
        {
            if (finish.m_IsPattern)
            {
                PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=basic_is_pattern id=" + finish.m_Id);
                return null;
            }

            string basicTexture = finish.GetSurfaceTexture(100);
            if (!IsProceduralColorTexture(basicTexture))
            {
                PaintZ_PaintLog.Warning("paint_pack_registry finish_rejected source=" + path + " reason=basic_surface_not_procedural_color id=" + finish.m_Id);
                return null;
            }
        }

        return finish;
    }

    protected static void ResolveFinishes(array<ref PaintZ_FinishDefinition> candidates)
    {
        TStringArray processed = new TStringArray;
        for (int i = 0; i < candidates.Count(); i++)
        {
            PaintZ_FinishDefinition candidate = candidates.Get(i);
            if (ContainsString(processed, candidate.m_Id))
                continue;

            processed.Insert(candidate.m_Id);
            int idMatches = 0;
            for (int j = 0; j < candidates.Count(); j++)
            {
                if (candidates.Get(j).m_Id == candidate.m_Id)
                    idMatches++;
            }

            if (idMatches != 1)
            {
                PaintZ_PaintLog.Warning("paint_pack_registry finish_conflict id=" + candidate.m_Id + " action=finish_disabled");
                continue;
            }

            int hashMatches = 0;
            int finishHash = candidate.m_Id.Hash();
            for (int k = 0; k < candidates.Count(); k++)
            {
                if (candidates.Get(k).m_Id.Hash() == finishHash)
                    hashMatches++;
            }

            if (hashMatches != 1)
            {
                PaintZ_PaintLog.Warning("paint_pack_registry network_hash_conflict id=" + candidate.m_Id + " hash=" + finishHash + " action=finish_disabled");
                continue;
            }

            s_Finishes.Insert(candidate);
            PaintZ_PaintLog.Info("paint_pack_registry finish_registered id=" + candidate.m_Id + " owner=" + candidate.m_OwnerClass + " source=" + candidate.m_Source);
        }
    }

    protected static bool IsNamespaceUsable(string prefix)
    {
        if (ContainsString(s_ConflictedPrefixes, prefix))
            return false;

        return GetNamespace(prefix) != null;
    }

    protected static bool ValidatePrefix(string prefix)
    {
        int length = prefix.Length();
        if (length < 2 || length > 3 || !IsUpperLetter(prefix.Get(0)))
            return false;

        for (int i = 1; i < length; i++)
        {
            string ch = prefix.Get(i);
            if (!IsUpperLetter(ch) && !IsDigit(ch))
                return false;
        }

        return true;
    }

    protected static bool IsReservedPrefix(string prefix)
    {
        return prefix.Length() >= 2 && prefix.Substring(0, 2) == "PZ";
    }

    protected static bool IsAssignedOfficialOwner(PaintZ_PaintPackNamespace owner)
    {
        if (!owner || !owner.m_Official)
            return false;

        return owner.m_Prefix == OFFICIAL_PZ_PREFIX && owner.m_ConfigClass == OFFICIAL_PZ_OWNER;
    }

    protected static bool ParseFinishId(string finishId, out string prefix, out string typeCode, out string suffix)
    {
        prefix = "";
        typeCode = "";
        suffix = "";

        TStringArray parts = new TStringArray;
        finishId.Split("-", parts);
        if (parts.Count() != 3)
            return false;

        prefix = parts.Get(0);
        typeCode = parts.Get(1);
        suffix = parts.Get(2);
        if (!ValidatePrefix(prefix) || typeCode.Length() != 1 || !IsTypeCode(typeCode))
            return false;

        int suffixLength = suffix.Length();
        if (suffixLength < 2 || suffixLength > 12)
            return false;

        for (int i = 0; i < suffixLength; i++)
        {
            string ch = suffix.Get(i);
            if (!IsUpperLetter(ch) && !IsDigit(ch))
                return false;
        }

        return true;
    }

    protected static bool IsTypeCode(string typeCode)
    {
        return typeCode == "B" || typeCode == "S" || typeCode == "C" || typeCode == "P" || typeCode == "M" || typeCode == "R" || typeCode == "W" || typeCode == "F" || typeCode == "X" || typeCode == "T";
    }

    protected static bool TypeMatchesCode(string typeName, string typeCode)
    {
        if (typeCode == "B")
            return typeName == "basic";
        if (typeCode == "S")
            return typeName == "solid";
        if (typeCode == "C")
            return typeName == "camo" || typeName == "camouflage";
        if (typeCode == "P")
            return typeName == "pattern";
        if (typeCode == "M")
            return typeName == "metallic";
        if (typeCode == "R")
            return typeName == "rusted" || typeName == "oxidized";
        if (typeCode == "W")
            return typeName == "weathered";
        if (typeCode == "F")
            return typeName == "fluorescent";
        if (typeCode == "X")
            return typeName == "special" || typeName == "custom";
        if (typeCode == "T")
            return typeName == "transparent" || typeName == "tint";

        return false;
    }

    protected static bool IsProceduralColorTexture(string texture)
    {
        string normalized = texture;
        normalized.ToLower();
        return normalized.IndexOf("#(argb,8,8,3)color(") == 0;
    }

    protected static bool IsUpperLetter(string ch)
    {
        return ch.Length() == 1 && "ABCDEFGHIJKLMNOPQRSTUVWXYZ".IndexOf(ch) >= 0;
    }

    protected static bool IsDigit(string ch)
    {
        return ch.Length() == 1 && "0123456789".IndexOf(ch) >= 0;
    }

    protected static bool ContainsString(TStringArray values, string value)
    {
        for (int i = 0; values && i < values.Count(); i++)
        {
            if (values.Get(i) == value)
                return true;
        }

        return false;
    }
};
