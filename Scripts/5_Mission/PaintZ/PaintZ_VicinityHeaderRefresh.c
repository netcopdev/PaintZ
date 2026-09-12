modded class ContainerWithCargo
{
    protected string m_PaintZLastVicinityHeaderName;

    override void UpdateInterval()
    {
        super.UpdateInterval();
        PaintZ_RefreshVicinityHeader();
    }

    protected void PaintZ_RefreshVicinityHeader()
    {
        if (!m_Entity)
            return;

        ItemBase item = ItemBase.Cast(m_Entity);
        if (!item)
            return;

        bool hasPaintState = item.PaintZ_HasState();
        if (!hasPaintState && m_PaintZLastVicinityHeaderName == "")
            return;

        string displayName = m_Entity.GetDisplayName();
        if (displayName == m_PaintZLastVicinityHeaderName)
            return;

        m_PaintZLastVicinityHeaderName = displayName;
        if (m_CargoGrid)
            m_CargoGrid.UpdateHeaderText();

        if (!hasPaintState)
            m_PaintZLastVicinityHeaderName = "";
    }
};

modded class ContainerWithCargoAndAttachments
{
    protected string m_PaintZLastVicinityHeaderName;

    override void UpdateInterval()
    {
        super.UpdateInterval();
        PaintZ_RefreshVicinityHeader();
    }

    protected void PaintZ_RefreshVicinityHeader()
    {
        if (!m_Entity)
            return;

        ItemBase item = ItemBase.Cast(m_Entity);
        if (!item)
            return;

        bool hasPaintState = item.PaintZ_HasState();
        if (!hasPaintState && m_PaintZLastVicinityHeaderName == "")
            return;

        string displayName = m_Entity.GetDisplayName();
        if (displayName == m_PaintZLastVicinityHeaderName)
            return;

        m_PaintZLastVicinityHeaderName = displayName;
        if (m_CargoGrid)
        {
            m_CargoGrid.UpdateHeaderText();
        }
        else if (m_ClosableHeader)
        {
            displayName.ToUpper();
            m_ClosableHeader.SetName(displayName);
        }

        if (!hasPaintState)
            m_PaintZLastVicinityHeaderName = "";
    }
};
