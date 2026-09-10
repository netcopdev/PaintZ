class PaintZ_StaleFinishRecoveryConfig
{
    int version;
    int reload_seconds = -999999;
    bool prune_unknown = false;
    ref map<string, string> migrations;

    void PaintZ_StaleFinishRecoveryConfig()
    {
        migrations = new map<string, string>;
    }
};
