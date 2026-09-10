class PaintZ_RuntimeDocs
{
    static const string PROFILE_DIRECTORY = "$profile:PaintZ";

    static void Refresh()
    {
        MakeDirectory(PROFILE_DIRECTORY);

        RefreshFile("PaintZ/config/paintz_items_README.txt", "$profile:PaintZ/paintz_items_README.txt", "item-policy");
        RefreshFile("PaintZ/config/paintz_pattern_scaling_README.txt", "$profile:PaintZ/paintz_pattern_scaling_README.txt", "pattern-scaling");
        RefreshFile("PaintZ/config/paintz_stale_finishes_README.txt", "$profile:PaintZ/paintz_stale_finishes_README.txt", "stale-finishes");
        RefreshFile("PaintZ/config/paintz_action_tuning_README.txt", "$profile:PaintZ/paintz_action_tuning_README.txt", "action-tuning");
    }

    protected static void RefreshFile(string bundledPath, string profilePath, string label)
    {
        if (FileExist(profilePath) && !DeleteFile(profilePath))
        {
            Print("[PaintZ][Docs][W] Could not remove old runtime " + label + " README path=" + profilePath);
            return;
        }

        if (!CopyFile(bundledPath, profilePath))
        {
            Print("[PaintZ][Docs][W] Could not refresh runtime " + label + " README from " + bundledPath);
            return;
        }

        Print("[PaintZ][Docs][I] refreshed runtime " + label + " README path=" + profilePath);
    }
};
