class PaintZ_PaintInspectionResult
{
    bool m_Paintable;
    int m_SelectionIndex;
    string m_SelectionName;
    string m_Reason;

    void PaintZ_PaintInspectionResult()
    {
        m_Paintable = false;
        m_SelectionIndex = -1;
        m_SelectionName = "";
        m_Reason = "Not inspected";
    }
};
