// Part of SimCoupe - A SAM Coupe emulator
//
// GUIDlg.cpp: Dialog boxes using the GUI controls
//
//  Copyright (c) 1999-2004  Simon Owen
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "SimCoupe.h"
#include "GUIDlg.h"

#include "CDisk.h"
#include "Frame.h"
#include "HardDisk.h"
#include "Input.h"
#include "Memory.h"
#include "Options.h"


OPTIONS g_opts;

// Helper macro for detecting options changes
#define Changed(o)         (g_opts.o != GetOption(o))
#define ChangedString(o)   (strcasecmp(g_opts.o, GetOption(o)))

////////////////////////////////////////////////////////////////////////////////

CAboutDialog::CAboutDialog (CWindow* pParent_/*=NULL*/)
    : CDialog(pParent_, 305, 220,  "About SimCoupe")
{
    new CIconControl(this, 6, 6, &sSamIcon);
    new CTextControl(this, 86, 10,  "SimCoupe v0.90 beta 10", BLACK);
    new CTextControl(this, 86, 24,  "http://www.simcoupe.org/", GREY_3);

    new CTextControl(this, 41, 46,  "Win32/SDL/Allegro/Pocket PC versions:", BLUE_5);
    new CTextControl(this, 51, 59,  "Simon Owen <simon.owen@simcoupe.org>", BLACK);

    new CTextControl(this, 41, 78,  "Based on original DOS/X versions by:", BLUE_5);
    new CTextControl(this, 51, 91,  "Allan Skillman <allan.skillman@arm.com>", BLACK);

    new CTextControl(this, 41, 110,  "Additional technical enhancements:", BLUE_5);
    new CTextControl(this, 51, 123,  "Dave Laundon <dave.laundon@simcoupe.org>", BLACK);

    new CTextControl(this, 41, 142,  "Phillips SAA 1099 sound chip emulation:", BLUE_5);
    new CTextControl(this, 51, 155,  "Dave Hooper <dave@rebuzz.org>", BLACK);

    new CTextControl(this, 41, 177, "See ReadMe.txt for additional information.", RED_3);

    m_pCloseButton = new CTextButton(this, (m_nWidth-55)/2, m_nHeight-21, "Close", 55);
}

void CAboutDialog::OnNotify (CWindow* pWindow_, int nParam_)
{
    if (pWindow_ == m_pCloseButton)
        Destroy();
}

void CAboutDialog::EraseBackground (CScreen* pScreen_)
{
    pScreen_->FillRect(m_nX, m_nY, m_nWidth, m_nHeight, WHITE);
}


////////////////////////////////////////////////////////////////////////////////

CFileDialog::CFileDialog (const char* pcszCaption_, const char* pcszPath_, const FILEFILTER* pcFileFilter_,
    CWindow* pParent_/*=NULL*/) : CDialog(pParent_, 527, 339, pcszCaption_), m_pcFileFilter(pcFileFilter_)
{
    // Create all the controls for the dialog (the objects are deleted by the GUI when closed)
    m_pFileView = new CFileView(this, 2, 2, (7*72)+19, (4*72));

    new CFrameControl(this, 0, (4*72)+3, m_nWidth, 1, GREY_6);

    new CTextControl(this, 3, m_nHeight-40,  "Path:");
    m_pPath = new CTextControl(this, 36, m_nHeight-40,  "");

    new CTextControl(this, 3, m_nHeight-19,  "Filter:");
    m_pFilter = new CComboBox(this, 36,m_nHeight-22, m_pcFileFilter->pcszDesc, 200);

    m_pShowHidden = new CCheckBox(this, 252, m_nHeight-19, "Show hidden files");

    m_pRefresh = new CTextButton(this, m_nWidth - 160, m_nHeight-21, "Refresh", 56);
    m_pOK = new CTextButton(this, m_nWidth - 99, m_nHeight-21, "OK", 46);
    m_pCancel = new CTextButton(this, m_nWidth - 50, m_nHeight-21, "Cancel", 46);

    // Set the filter and path
    OnNotify(m_pFilter,0);
    m_pFileView->SetPath(pcszPath_);
}

// Handle control notifications
void CFileDialog::OnNotify (CWindow* pWindow_, int nParam_)
{
    if (pWindow_ == m_pOK)
        m_pFileView->NotifyParent(1);
    else if (pWindow_ == m_pCancel)
        Destroy();
    else if (pWindow_ == m_pRefresh)
        m_pFileView->Refresh();
    else if (pWindow_ == m_pShowHidden)
        m_pFileView->ShowHidden(m_pShowHidden->IsChecked());
    else if (pWindow_ == m_pFilter)
        m_pFileView->SetFilter(m_pcFileFilter->pcszExts[m_pFilter->GetSelected()]);
    else if (pWindow_ == m_pFileView)
    {
        const CListViewItem* pItem = m_pFileView->GetItem();
        if (pItem)
        {
            // Folder notifications simply update the displayed path
            if (pItem->m_pIcon == &sFolderIcon)
                m_pPath->SetText(m_pFileView->GetPath());

            // Opening/double-clicking the file requires custom handling
            else if (nParam_)
                OnOK();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

// File filter for disk images
static const FILEFILTER sFloppyFilter =
{
#ifdef USE_ZLIB
    "All Disks (.dsk;.sad;.sdf;.sbt; .gz;.zip)|"
    "Disk Images (.dsk;.sad;.sdf;.sbt)|"
    "Compressed Files (.gz;.zip)|"
    "All Files",

    {
        ".dsk;.sad;.sdf;.sbt;.gz;.zip",
        ".dsk;.sad;.sdf;.sbt",
        ".gz;.zip",
        ""
    }
#else
    "All Disks (.dsk;.sad;.sdf;.sbt)|"
    "Disk Images (.dsk;.sad;.sdf;.sbt)|"
    "All Files",

    {
        ".dsk;.sad;.sdf;.sbt",
        ".dsk;.sad;.sdf;.sbt",
        ""
    }
#endif
};

CInsertFloppy::CInsertFloppy (int nDrive_, CWindow* pParent_/*=NULL*/)
    : CFileDialog("", NULL, &sFloppyFilter, pParent_), m_nDrive(nDrive_)
{
    // Set the dialog caption to show which drive we're dealing with
    char szCaption[32] = "Insert Floppy x";
    szCaption[strlen(szCaption)-1] = '0'+nDrive_;
    SetText(szCaption);

    // Browse from the location of the previous image, or the default directory if none
    const char* pcszImage = ((nDrive_ == 1) ? pDrive1 : pDrive2)->GetImage();
    m_pFileView->SetPath(*pcszImage ? pcszImage : OSD::GetFilePath());
}

// Handle OK being clicked when a file is selected
void CInsertFloppy::OnOK ()
{
    const char* pcszPath = m_pFileView->GetFullPath();

    if (pcszPath)
    {
        bool fInserted = false;

        // Insert the disk into the appropriate drive
        if (m_nDrive == 1)
            fInserted = pDrive1->Insert(SetOption(disk1, pcszPath));
        else
            fInserted = pDrive2->Insert(SetOption(disk2, pcszPath));

        // If we succeeded, show a status message and close the file selector
        if (fInserted)
        {
            // Update the status text and close the dialog
            Frame::SetStatus("%s  inserted into Drive %d", m_pFileView->GetItem()->m_pszLabel, m_nDrive);
            Destroy();
            return;
        }
    }

    // Report any error
    char szBody[MAX_PATH+32];
    sprintf(szBody, "Invalid disk image:\n\n%s", m_pFileView->GetItem()->m_pszLabel);
    new CMessageBox(this, szBody, "Open Failed", mbWarning);
}

////////////////////////////////////////////////////////////////////////////////

CFileBrowser::CFileBrowser (CEditControl* pEdit_, CWindow* pParent_, const char* pcszCaption_, const FILEFILTER* pcsFilter_)
    : CFileDialog(pcszCaption_, NULL, pcsFilter_, pParent_), m_pEdit(pEdit_)
{
    // Browse from the location of the previous image, or the default directory if none
    m_pFileView->SetPath(*pEdit_->GetText() ? pEdit_->GetText() : OSD::GetFilePath());
}

// Handle OK being clicked when a file is selected
void CFileBrowser::OnOK ()
{
    const char* pcszPath = m_pFileView->GetFullPath();

    if (pcszPath)
    {
        Destroy();

        // Set the edit control text, activate it, and notify the parent of the change
        m_pEdit->SetText(pcszPath);
        m_pEdit->Activate();
        m_pParent->OnNotify(m_pEdit,0);
    }
}

////////////////////////////////////////////////////////////////////////////////

CHDDProperties::CHDDProperties (CEditControl* pEdit_, CWindow* pParent_, const char* pcszCaption_)
    : CDialog(pParent_, 268, 170, pcszCaption_), m_pEdit(pEdit_)
{
    new CTextControl(this, 12, 13,  "File:");
    m_pFile = new CEditControl(this, 35, 10, 199, pEdit_->GetText());
    m_pBrowse = new CTextButton(this, 239, 10, "...", 17);

    new CFrameControl(this, 12, 37, 244, 100);
    new CTextControl(this, 17, 33, "Geometry", YELLOW_8, BLUE_2);

    new CTextControl(this, 57, 53,  "Cylinders (1-16383):");
    new CTextControl(this, 89, 73,  "Heads (2-16):");
    new CTextControl(this, 82, 93,  "Sectors (1-63):");
    new CTextControl(this, 81, 113, "Total size (MB):");

    m_pCyls = new CEditControl(this, 167, 50,  40);
    m_pHeads = new CEditControl(this, 167, 70,  20);
    m_pSectors = new CEditControl(this, 167, 90,  20);
    m_pSize = new CEditControl(this, 167, 110, 30);

    m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
    m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

    // Force a refresh of the geometry from the current image (if any)
    OnNotify(m_pFile,0);
}

void CHDDProperties::OnNotify (CWindow* pWindow_, int nParam_)
{
    static const FILEFILTER sHardDiskFilter =
    {
        "Hard disk images (*.hdf)|"
        "All Files",

        { ".hdf", "" }
    };


    if (pWindow_ == m_pCancel)
        Destroy();
    else if (pWindow_ == m_pBrowse)
        new CFileBrowser(m_pFile, this, "Browse for HDF", &sHardDiskFilter);
    else if (pWindow_ == m_pFile)
    {
        // If we can, open the existing hard disk image to retrieve the geometry
        CHardDisk* pDisk = CHardDisk::OpenObject(m_pFile->GetText());
        bool fExists = pDisk != NULL;

        if (fExists)
        {
            // Fetch the existing disk geometry
            HARDDISK_GEOMETRY geom;
            pDisk->GetGeometry (&geom);
            delete pDisk;

            // Initialise the edit controls with the current values
            m_pCyls->SetValue(geom.uCylinders);
            m_pHeads->SetValue(geom.uHeads);
            m_pSectors->SetValue(geom.uSectors);
            m_pSize->SetValue((geom.uTotalSectors + (1<<11)-1) >> 11);
        }

        // The geometry is read-only for existing images
        m_pCyls->Enable(!fExists);
        m_pHeads->Enable(!fExists);
        m_pSectors->Enable(!fExists);
        m_pSize->Enable(!fExists);

        // Set the text and state of the OK button, depending on the target file
        m_pOK->SetText(fExists ? "OK" : "Create");
        m_pOK->Enable(!!*m_pFile->GetText());
    }
    else if (pWindow_ == m_pCyls || pWindow_ == m_pHeads || pWindow_ == m_pSectors)
    {
        // Set the new size from the modified geometry
        UINT uSize = ((m_pCyls->GetValue()*m_pHeads->GetValue()*m_pSectors->GetValue()) + (1<<11)-1) >> 11;
        m_pSize->SetValue(uSize);
    }
    else if (pWindow_ == m_pSize)
    {
        // Fetch the updates size value
        UINT uSize = m_pSize->GetValue();

        // Set a disk geometry matching the new size
        m_pCyls->SetValue((uSize << 2) & 0x3fff);
        m_pHeads->SetValue(16);
        m_pSectors->SetValue(32);
    }
    else if (pWindow_ == m_pOK)
    {
        // Fetch the geometry values
        UINT uCyls = m_pCyls->GetValue();
        UINT uHeads = m_pHeads->GetValue();
        UINT uSectors = m_pSectors->GetValue();

        // Check the geometry is within range, since the edit fields can be modified directly
        if (!uCyls || (uCyls > 16383) || !uHeads || (uHeads > 16) || !uSectors || (uSectors > 63))
        {
            new CMessageBox(this, "Invalid disk geometry.", "Warning", MB_OK|MB_ICONEXCLAMATION);
            return;
        }

        // Create the new HDF image
        if (!CHDFHardDisk::Create(m_pFile->GetText(), uCyls, uHeads, uSectors))
        {
            new CMessageBox(this, "Failed to create new disk (disk full?)", "Warning", MB_OK|MB_ICONEXCLAMATION);
            return;
        }

        Destroy();
        m_pEdit->SetText(m_pFile->GetText());
        m_pEdit->Activate();
        m_pParent->OnNotify(m_pEdit,0);
    }
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

CTestDialog::CTestDialog (CWindow* pParent_/*=NULL*/)
    : CDialog(pParent_, 205, 198, "GUI Test")
{
    memset(m_apControls, 0, sizeof m_apControls);

    m_apControls[0] = new CEditControl(this, 8, 8, 190, "Edit control");

    m_apControls[1] = new CCheckBox(this, 8, 38, "Checked check-box");
    m_apControls[2] = new CCheckBox(this, 8, 54, "Unchecked check-box");
    reinterpret_cast<CCheckBox*>(m_apControls[1])->SetChecked();

    m_apControls[3] = new CRadioButton(this, 8, 78, "First option");
    m_apControls[4] = new CRadioButton(this, 8, 94, "Second option");
    m_apControls[5] = new CRadioButton(this, 8, 110, "Third option");
    reinterpret_cast<CRadioButton*>(m_apControls[3])->Select();

    m_apControls[6] = new CComboBox(this, 105,78, "Coch|Gwyn|Glas|Melyn", 70);
    m_apControls[7] = new CTextButton(this, 105, 103, "Button", 50);
    m_apControls[8] = new CScrollBar(this, 183,38,110,400);

    m_apControls[9] = new CEditControl(this, 130, 133, 20, "0");

    m_apControls[11] = new CIconControl(this, 8, 133, &sErrorIcon);

    m_apControls[12] = new CTextControl(this, 40, 133, "<- Icon control");
    m_apControls[13] = new CTextControl(this, 45, 149, "Coloured text control", GREEN_7);

    m_pEnable = new CCheckBox(this, 8, m_nHeight-20, "Controls enabled");
    reinterpret_cast<CCheckBox*>(m_pEnable)->SetChecked();

    m_pClose = new CTextButton(this, m_nWidth-55, m_nHeight-22, "Close", 50);

    m_pEnable->Activate();
}

void CTestDialog::OnNotify (CWindow* pWindow_, int nParam_)
{
    if (pWindow_ == m_pClose)
        Destroy();
    else if (pWindow_ == m_pEnable)
    {
        bool fIsChecked = reinterpret_cast<CCheckBox*>(m_pEnable)->IsChecked();

        // Update the enabled/disabled state of the control so we can see what they look like
        for (int i = 0 ; i < (sizeof m_apControls / sizeof m_apControls[0]) ; i++)
        {
            if (m_apControls[i])
                m_apControls[i]->Enable(fIsChecked);
        }
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////


COptionsDialog::COptionsDialog (CWindow* pParent_/*=NULL*/)
    : CDialog(pParent_, 364, 171, "Options")
{
    Move(m_nX, m_nY-40);

    m_pOptions = new COptionView(this, 2, 2, 360, 144);
    new CFrameControl(this, 0, m_nHeight-23, m_nWidth, 1);
    m_pStatus = new CTextControl(this, 4, m_nHeight-15, "", GREY_7);
    m_pClose = new CTextButton(this, m_nWidth-57, m_nHeight-19, "Close", 55);

    CListViewItem* pItem = NULL;

    // Add icons in reverse order
    pItem = new CListViewItem(&sSamIcon, "About", pItem);
    pItem = new CListViewItem(&sHardwareIcon, "Misc", pItem);
    pItem = new CListViewItem(&sPortIcon, "Parallel", pItem);
    pItem = new CListViewItem(&sFloppyDriveIcon, "Disks", pItem);
    pItem = new CListViewItem(&sHardDiskIcon, "Drives", pItem);
    pItem = new CListViewItem(&sKeyboardIcon, "Input", pItem);
    pItem = new CListViewItem(&sMidiIcon, "MIDI", pItem);
    pItem = new CListViewItem(&sSoundIcon, "Sound", pItem);
    pItem = new CListViewItem(&sDisplayIcon, "Display", pItem);
    pItem = new CListViewItem(&sChipIcon, "System", pItem);
    m_pOptions->SetItems(pItem);

    // Set the initial status text
    OnNotify(m_pOptions,0);
}


class CSystemOptions : public CDialog
{
    public:
        CSystemOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 241, "System Settings")
        {
            new CIconControl(this, 10, 10, &sChipIcon);

            new CFrameControl(this, 50, 17, 238, 45);
            new CTextControl(this, 60, 13, "Memory", YELLOW_8, BLUE_2);

            new CTextControl(this, 63, 35, "Main:");
            m_pMain = new CComboBox(this, 93, 32, "256K|512K", 60);
            new CTextControl(this, 167, 35, "External:");
            m_pExternal = new CComboBox(this, 217, 32, "None|1MB|2MB|3MB|4MB", 60);

            new CFrameControl(this, 50, 80, 238, 80);
            new CTextControl(this, 60, 76, "System ROM", YELLOW_8, BLUE_2);

            new CTextControl(this, 63, 96, "Custom ROM image (32K):");
            m_pROM = new CEditControl(this, 63, 113, 196);
            m_pBrowse = new CTextButton(this, 262, 113, "...", 17);

            m_pFastReset = new CCheckBox(this, 63, 139, "Enable fast power-on ROM reset.");

            new CTextControl(this, 50, 174, "Note: changes to the settings above require", GREY_7);
            new CTextControl(this, 50, 189, "a SAM reset to take effect.", GREY_7);

            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

            // Set the initial state from the options
            m_pMain->Select((GetOption(mainmem) >> 8) - 1);
            m_pExternal->Select(GetOption(externalmem));
            m_pROM->SetText(GetOption(rom));
            m_pFastReset->SetChecked(GetOption(fastreset));

            // Update the state of the controls to reflect the current settings
            OnNotify(m_pMain,0);
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {
            static const FILEFILTER sROMFilter =
            {
                "ROM Images (.rom;.bin)|"
                "All Files",

                { ".rom;.bin", "" }
            };

            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pBrowse)
                new CFileBrowser(m_pROM, this, "Browse for ROM", &sROMFilter);
            else if (pWindow_ == m_pOK)
            {
                SetOption(mainmem, (m_pMain->GetSelected()+1) << 8);
                SetOption(externalmem, m_pExternal->GetSelected());
                SetOption(rom, m_pROM->GetText());
                SetOption(fastreset, m_pFastReset->IsChecked());

                Destroy();
            }
        }

    protected:
        CCheckBox *m_pFastReset;
        CComboBox *m_pMain, *m_pExternal;
        CEditControl *m_pROM;
        CTextButton *m_pOK, *m_pCancel, *m_pBrowse;
};


class CDisplayOptions : public CDialog
{
    public:
        CDisplayOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 231, "Display Settings")
        {
            new CIconControl(this, 10, 10, &sDisplayIcon);

            new CFrameControl(this, 50, 17, 238, 185, WHITE);
            new CTextControl(this, 60, 13, "Settings", WHITE, BLUE_2);

            m_pFullScreen = new CCheckBox(this, 60, 35, "Full-screen");

                m_pScaleText = new CTextControl(this, 85, 57, "Windowed mode scale:");
                m_pScale = new CComboBox(this, 215, 54, "0.5x|1x|1.5x", 50);
                m_pDepthText = new CTextControl(this, 85, 79, "Full-screen colour depth:");
                m_pDepth = new CComboBox(this, 215, 76, "8-bit|16-bit|32-bit", 60);

            new CFrameControl(this, 63, 102, 212, 1, GREY_6);

            m_pStretch = new CCheckBox(this, 60, 115, "Stretch to fit");
            m_pSync = new CCheckBox(this, 60, 136, "Sync to 50Hz");
            m_pAutoFrameSkip = new CCheckBox(this, 60, 157, "Auto frame-skip");
            m_pScanlines = new CCheckBox(this, 165, 115, "Display scanlines");
            m_pRatio54 = new CCheckBox(this, 165, 136, "5:4 pixel shape");
            m_pFrameSkip = new CComboBox(this, 165, 154, "Show ALL frames|Show every 2nd|Show every 3rd|Show every 4th|Show every 5th|Show every 6th|Show every 7th|Show every 8th|Show every 9th|Show every 10th", 115);

            new CTextControl(this, 60, 180, "Viewable area:");
            m_pViewArea = new CComboBox(this, 140, 177, "No borders|Small borders|Short TV area (default)|TV visible area|Complete scan area", 140);

            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

            // Set the initial state from the options
            int anDepths[] = { 0, 1, 2, 2 };
            m_pDepth->Select(anDepths[((GetOption(depth) >> 3) - 1) & 3]);
            m_pScale->Select(GetOption(scale)-1);

            m_pFullScreen->SetChecked(GetOption(fullscreen) != 0);
            m_pSync->SetChecked(GetOption(sync) != 0);
            m_pRatio54->SetChecked(GetOption(ratio5_4));
            m_pStretch->SetChecked(GetOption(stretchtofit));

            bool fScanlines = GetOption(scanlines) && !GetOption(stretchtofit);
            m_pScanlines->SetChecked(fScanlines);

            m_pAutoFrameSkip->SetChecked(!GetOption(frameskip));
            m_pFrameSkip->Select(GetOption(frameskip) ? GetOption(frameskip)-1 : 0);
            m_pViewArea->Select(GetOption(borders));

            // Update the state of the controls to reflect the current settings
            OnNotify(m_pScale,0);
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {
            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pOK)
            {
                SetOption(fullscreen, m_pFullScreen->IsChecked());

                int anDepths[] = { 8, 16, 32 };
                SetOption(depth, anDepths[m_pDepth->GetSelected()]);
                SetOption(scale, m_pScale->GetSelected()+1);

                SetOption(sync, m_pSync->IsChecked());
                SetOption(ratio5_4, m_pRatio54->IsChecked());
                SetOption(stretchtofit, m_pStretch->IsChecked());
                SetOption(scanlines, m_pScanlines->IsChecked());

                int nFrameSkip = !m_pAutoFrameSkip->IsChecked();
                SetOption(frameskip, nFrameSkip ? m_pFrameSkip->GetSelected()+1 : 0);

                SetOption(borders, m_pViewArea->GetSelected());

                if (Changed(borders) || Changed(fullscreen) || Changed(ratio5_4) || (GetOption(fullscreen) && Changed(depth)))
                {
                    Frame::Init();

                    // Re-centre the window, including the parent if that's a dialog
                    if (GetParent()->GetType() == ctDialog)
                        reinterpret_cast<CDialog*>(GetParent())->Centre();
                    Centre();
                }

                Destroy();
            }
            else
            {
                bool fFullScreen = m_pFullScreen->IsChecked();
                m_pScaleText->Enable(!fFullScreen);
                m_pScale->Enable(!fFullScreen);
                m_pDepthText->Enable(fFullScreen);
                m_pDepth->Enable(fFullScreen);

                m_pFrameSkip->Enable(!m_pAutoFrameSkip->IsChecked());

                // SDL doesn't allow certain features to be changed at present
                m_pScaleText->Enable(false);
                m_pScale->Enable(false);
                m_pStretch->Enable(false);

#if defined(SDL) || defined(ALLEGRO_DOS)
                m_pScanlines->Enable(false);
                m_pRatio54->Enable(false);
#endif
            }
        }

    protected:
        CCheckBox *m_pFullScreen, *m_pStretch, *m_pSync, *m_pAutoFrameSkip, *m_pScanlines, *m_pRatio54;
        CComboBox *m_pScale, *m_pDepth, *m_pFrameSkip, *m_pViewArea;
        CTextControl *m_pScaleText, *m_pDepthText;
        CTextButton *m_pOK, *m_pCancel;
};


class CSoundOptions : public CDialog
{
    public:
        CSoundOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 231, "Sound Settings")
        {
            new CIconControl(this, 10, 10, &sSoundIcon);
            new CFrameControl(this, 50, 17, 238, 185, WHITE);

            m_pSound = new CCheckBox(this, 60, 13, "Sound enabled", WHITE, BLUE_2);

                m_pSAA = new CCheckBox(this, 70, 35, "Enable Philips SAA 1099 sound chip");
                m_pBeeper = new CCheckBox(this, 70, 56, "Enable Spectrum-style beeper");

            new CFrameControl(this, 70, 84, 208, 100, WHITE);
            new CTextControl(this, 80, 80, "Output", WHITE, BLUE_2);

                m_pFreqText = new CTextControl(this, 90, 97, "Frequency:");
                m_pFreq = new CComboBox(this, 158, 94, "11025 Hz|22050 Hz|44100 Hz", 75);
                m_pSampleSizeText = new CTextControl(this, 90, 119, "Sample size:");
                m_pSampleSize = new CComboBox(this, 158, 116, "8-bit|16-bit", 60);
                m_pLatencyText = new CTextControl(this, 90, 141, "Latency:");
                m_pLatency = new CComboBox(this, 158, 138, "1 frame (best)|2 frames|3 frames|4 frames|5 frames (default)|10 frames|15 frames|20 frames|25 frames", 113);
                m_pStereo = new CCheckBox(this, 90, 162, "Stereo output");


            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

            // Set the initial state from the options
            m_pSound->SetChecked(GetOption(sound));
            m_pSAA->SetChecked(GetOption(saasound));
            m_pBeeper->SetChecked(GetOption(beeper));

            m_pFreq->Select(GetOption(freq)/11025 - 1);
            m_pSampleSize->Select((GetOption(bits) >> 3)-1);
            m_pStereo->SetChecked(GetOption(stereo));

            int nLatency = GetOption(latency);
            m_pLatency->Select((nLatency <= 5 ) ? nLatency - 1 : nLatency/5 + 3);

            // Update the state of the controls to reflect the current settings
            OnNotify(m_pSound,0);
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {
            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pOK)
            {
                SetOption(sound, m_pSound->IsChecked());
                SetOption(saasound, m_pSAA->IsChecked());
                SetOption(beeper, m_pBeeper->IsChecked());

                SetOption(freq, 11025 * (1 << m_pFreq->GetSelected()));
                SetOption(bits, (m_pSampleSize->GetSelected()+1) << 3);
                SetOption(stereo, m_pStereo->IsChecked());

                int nLatency = m_pLatency->GetSelected();
                SetOption(latency, (nLatency < 5) ? nLatency + 1 : (nLatency - 3) * 5);

                if (Changed(sound) || Changed(saasound) || Changed(beeper) ||
                     Changed(freq) || Changed(bits) || Changed(stereo) || Changed(latency))
                    Sound::Init();

                if (Changed(beeper))
                    IO::InitBeeper();

                // If the sound was checked but the option isn't set, warn than it failed
                if (m_pSound->IsChecked() && !GetOption(sound))
                    new CMessageBox(GetParent(), "Sound init failed - device in use?", "Sound", mbWarning);

                Destroy();
            }
            else
            {
                bool fSound = m_pSound->IsChecked();

                m_pFreqText->Enable(fSound);
                m_pFreq->Enable(fSound);
                m_pSampleSizeText->Enable(fSound);
                m_pSampleSize->Enable(fSound);

                m_pSAA->Enable(fSound);
                m_pBeeper->Enable(fSound);
                m_pStereo->Enable(fSound);
                m_pLatencyText->Enable(fSound);
                m_pLatency->Enable(fSound);

#ifndef USE_SAASOUND
                m_pSAA->Enable(false);
#endif
            }
        }

    protected:
        CCheckBox *m_pSound, *m_pSAA, *m_pBeeper, *m_pStereo;
        CComboBox *m_pFreq, *m_pSampleSize, *m_pLatency;
        CTextControl *m_pLatencyText, *m_pFreqText, *m_pSampleSizeText;
        CTextButton *m_pOK, *m_pCancel;
};


class CMidiOptions : public CDialog
{
    public:
        CMidiOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 241, "Midi Settings")
        {
            new CIconControl(this, 10, 15, &sMidiIcon);
            new CFrameControl(this, 50, 17, 238, 40);
            new CTextControl(this, 60, 13, "Active Device", YELLOW_8, BLUE_2);
            new CTextControl(this, 63, 33, "Device on MIDI port:");
            m_pMidi = new CComboBox(this, 170, 30, "None|Midi device|Network", 90);

            new CFrameControl(this, 50, 72, 238, 68);
            new CTextControl(this, 60, 68, "Devices", YELLOW_8, BLUE_2);

            new CTextControl(this, 63, 88, "MIDI Out:");
            m_pMidiOut = new CComboBox(this, 115, 85, "/dev/midi", 160);

            new CTextControl(this, 63, 115, "MIDI In:");
            m_pMidiIn = new CComboBox(this, 115, 113, "/dev/midi", 160);

            new CFrameControl(this, 50, 155, 238, 40);
            new CTextControl(this, 60, 151, "Network (not currently supported)", YELLOW_8, BLUE_2);

            new CTextControl(this, 63, 171, "Station ID:");
            m_pStationId = new CEditControl(this, 120, 168, 20, "0");

            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

            // Set the initial state from the options
            m_pMidi->Select(GetOption(midi));

            // Update the state of the controls to reflect the current settings
            OnNotify(m_pMidi,0);
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {
            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pOK)
            {
                SetOption(midi, m_pMidi->GetSelected());
                SetOption(midioutdev, m_pMidiOut->GetSelectedText());
                SetOption(midiindev, m_pMidiIn->GetSelectedText());

                if (Changed(midi) || Changed(midiindev) || Changed(midioutdev))
                    IO::InitMidi();

                Destroy();
            }
            else
            {
                int nType = m_pMidi->GetSelected();
                m_pMidiOut->Enable(nType == 1);
                m_pMidiIn->Enable(nType == 1);
                m_pStationId->Enable(nType == 2);

                m_pStationId->Enable(false);
            }
        }

    protected:
        CComboBox *m_pMidi, *m_pMidiOut, *m_pMidiIn;
        CEditControl *m_pStationId;
        CTextButton *m_pOK, *m_pCancel;
};


class CInputOptions : public CDialog
{
    public:
        CInputOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 241, "Input Settings")
        {
            new CIconControl(this, 10, 10, &sKeyboardIcon);
            new CFrameControl(this, 50, 17, 238, 91);
            new CTextControl(this, 60, 13, "Keyboard", YELLOW_8, BLUE_2);

            new CTextControl(this, 63, 35, "Mapping mode:");
            m_pKeyMapping = new CComboBox(this, 145, 32, "None (raw)|SAM Coupe|Sinclair Spectrum", 115);

            m_pAltForCntrl = new CCheckBox(this, 63, 63, "Use Left-Alt for SAM Cntrl key.");
            m_pAltGrForEdit = new CCheckBox(this, 63, 85, "Use Alt-Gr for SAM Edit key.");

            new CIconControl(this, 10, 123, &sMouseIcon);
            new CFrameControl(this, 50, 125, 238, 37);
            new CTextControl(this, 60, 121, "Mouse", YELLOW_8, BLUE_2);

            m_pMouse = new CCheckBox(this, 63, 138, "Enable SAM mouse interface.");

            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

            // Set the initial state from the options
            m_pKeyMapping->Select(GetOption(keymapping));
            m_pAltForCntrl->SetChecked(GetOption(altforcntrl));
            m_pAltGrForEdit->SetChecked(GetOption(altgrforedit));
            m_pMouse->SetChecked(GetOption(mouse));

            // Update the state of the controls to reflect the current settings
            OnNotify(m_pMouse,0);
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {
            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pOK)
            {
                SetOption(keymapping, m_pKeyMapping->GetSelected());
                SetOption(altforcntrl, m_pAltForCntrl->IsChecked());
                SetOption(altgrforedit, m_pAltGrForEdit->IsChecked());
                SetOption(mouse, m_pMouse->IsChecked());

                if (Changed(keymapping) || Changed(mouse))
                    Input::Init();

                Destroy();
            }
        }

    protected:
        CComboBox *m_pKeyMapping;
        CCheckBox *m_pAltForCntrl, *m_pAltGrForEdit, *m_pMouse;
        CTextButton *m_pOK, *m_pCancel;
};


class CDriveOptions : public CDialog
{
    public:
        CDriveOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 241, "Drive Settings")
        {
            new CIconControl(this, 10, 10, &sHardDiskIcon);

            new CFrameControl(this, 50, 16, 238, 38);
            new CTextControl(this, 60, 12, "Drive 1", YELLOW_8, BLUE_2);

            new CTextControl(this, 63, 30, "Device connected:");
            m_pDrive1 = new CComboBox(this, 163, 27, "None|Floppy disk image", 115);

            new CFrameControl(this, 50, 64, 238, 38);
            new CTextControl(this, 60, 60, "Drive 2", YELLOW_8, BLUE_2);

            new CTextControl(this, 63, 78, "Device connected:");
            m_pDrive2 = new CComboBox(this, 163, 75, "None|Floppy disk image|Atom hard disk", 115);

            new CFrameControl(this, 50, 112, 238, 35);
            new CTextControl(this, 60, 108, "Options", YELLOW_8, BLUE_2);

            m_pTurboLoad = new CCheckBox(this, 60, 125, "Fast disk access");
            new CTextControl(this, 165, 126, "Sensitivity:");
            m_pSensitivity = new CComboBox(this, 220, 122, "Low|Medium|High", 62);

            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

            // Set the initial state from the options
            m_pDrive1->Select(GetOption(drive1));
            m_pDrive2->Select(GetOption(drive2));
            m_pTurboLoad->SetChecked(GetOption(turboload) != 0);
            m_pSensitivity->Select(!GetOption(turboload) ? 1 : GetOption(turboload) <= 5 ? 2 :
                                                               GetOption(turboload) <= 50 ? 1 : 0);

            // Update the state of the controls to reflect the current settings
            OnNotify(m_pTurboLoad,0);
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {
            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pOK)
            {
                int anDriveTypes[] = { dskNone, dskImage, dskImage, dskAtom };
                SetOption(drive1, anDriveTypes[m_pDrive1->GetSelected()]);
                SetOption(drive2, anDriveTypes[m_pDrive2->GetSelected()]);

                int anSpeeds[] = { 100, 50, 5 };
                SetOption(turboload, m_pTurboLoad->IsChecked() ? anSpeeds[m_pSensitivity->GetSelected()] : 0);

                if (Changed(drive1) || Changed(drive2))
                    IO::InitDrives();

                Destroy();
            }
            else if (pWindow_ == m_pTurboLoad)
                m_pSensitivity->Enable(m_pTurboLoad->IsChecked());
        }

    protected:
        CComboBox *m_pDrive1, *m_pDrive2, *m_pSensitivity;
        CCheckBox* m_pTurboLoad;
        CTextButton *m_pOK, *m_pCancel;
};


class CDiskOptions : public CDialog
{
    public:
        CDiskOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 241, "Disk Settings")
        {
            new CIconControl(this, 10, 10, &sFloppyDriveIcon);

            new CFrameControl(this, 50, 10, 238, 34);
            new CTextControl(this, 60, 6, "Floppy Drive 1", YELLOW_8, BLUE_2);
            m_pFloppy1 = new CEditControl(this, 60, 20, 200, GetOption(disk1));
            m_pBrowseFloppy1 = new CTextButton(this, 264, 20, "...", 17);

            new CFrameControl(this, 50, 53, 238, 34);
            new CTextControl(this, 60, 49, "Floppy Drive 2", YELLOW_8, BLUE_2);
            m_pFloppy2 = new CEditControl(this, 60, 63, 200, GetOption(disk2));
            m_pBrowseFloppy2 = new CTextButton(this, 264, 63, "...", 17);

            new CFrameControl(this, 50, 96, 238, 34);
            new CTextControl(this, 60, 92, "Atom Hard Disk", YELLOW_8, BLUE_2);
            m_pAtom = new CEditControl(this, 60, 106, 200, GetOption(atomdisk));
            m_pBrowseAtom = new CTextButton(this, 264, 106, "...", 17);

            new CFrameControl(this, 50, 139, 238, 34);
            new CTextControl(this, 60, 135, "SD-IDE Hard Disk", YELLOW_8, BLUE_2);
            m_pSDIDE = new CEditControl(this, 60, 149, 200, GetOption(sdidedisk));
            m_pBrowseSDIDE = new CTextButton(this, 264, 149, "...", 17);

            new CFrameControl(this, 50, 182, 238, 34);
            new CTextControl(this, 60, 178, "YAMOD.ATBUS Hard Disk", YELLOW_8, BLUE_2);
            m_pYATBus = new CEditControl(this, 60, 192, 200, GetOption(yatbusdisk));
            m_pBrowseYATBus = new CTextButton(this, 264, 192, "...", 17);

            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {

            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pOK)
            {
                char sz[MAX_PATH+128]="";

                // Set the options from the edit control values
                SetOption(disk1, m_pFloppy1->GetText());
                SetOption(disk2, m_pFloppy2->GetText());
                SetOption(atomdisk, m_pAtom->GetText());
                SetOption(sdidedisk, m_pSDIDE->GetText());
                SetOption(yatbusdisk, m_pYATBus->GetText());

                if (ChangedString(disk1) && !pDrive1->Insert(GetOption(disk1)))
                {
                    sprintf(sz, "Invalid disk image:\n\n%s", GetOption(disk1));
                    new CMessageBox(this, sz, "Floppy Drive 1", mbWarning);
                    SetOption(disk1, pDrive1->GetImage());
                    return;
                }

                if (ChangedString(disk2) && GetOption(drive2) == dskImage && !pDrive2->Insert(GetOption(disk2)))
                {
                    sprintf(sz, "Invalid disk image:\n\n%s", GetOption(disk2));
                    new CMessageBox(this, sz, "Floppy Drive 2", mbWarning);
                    SetOption(disk2, pDrive2->GetImage());
                    return;
                }

                // If the Atom path has changed, activate it
                if (ChangedString(atomdisk))
                {
                    // If the Atom is active, force it to be remounted
                    if (GetOption(drive2) == dskAtom)
                    {
                        delete pDrive2;
                        pDrive2 = NULL;
                    }

                    // Force the type of drive 2 as appropriate for the new string
                    SetOption(drive2, *GetOption(atomdisk) ? dskAtom : dskImage);
                    IO::InitDrives();

                    // Ensure it was mounted ok
                    if (*GetOption(atomdisk) && pDrive2->GetType() != dskAtom)
                    {
                        sprintf(sz, "Invalid hard disk image:\n\n%s", GetOption(atomdisk));
                        new CMessageBox(this, sz, "Atom Disk", mbWarning);
                        SetOption(atomdisk, "");
                        return;
                    }
                }

                // Re-init the other hard drive interfaces if anything has changed
                if (ChangedString(sdidedisk) || ChangedString(yatbusdisk))
                    IO::InitHDD();

                // If the SDIDE path changed, check it was mounted ok
                if (ChangedString(sdidedisk) && *GetOption(sdidedisk) && pSDIDE->GetType() != dskSDIDE)
                {
                    sprintf(sz, "Invalid hard disk image:\n\n%s", GetOption(sdidedisk));
                    new CMessageBox(this, sz, "SDIDE Disk", mbWarning);
                    SetOption(sdidedisk, "");
                    return;
                }

                // If the SDIDE path changed, check it was mounted ok
                if (ChangedString(yatbusdisk) && *GetOption(yatbusdisk) && pYATBus->GetType() != dskYATBus)
                {
                    sprintf(sz, "Invalid hard disk image:\n\n%s", GetOption(yatbusdisk));
                    new CMessageBox(this, sz, "YAMOD.ATBUS Disk", mbWarning);
                    SetOption(yatbusdisk, "");
                    return;
                }

                // If everything checked out, close the dialog
                Destroy();
            }
            else if (pWindow_ == m_pBrowseFloppy1)
                new CFileBrowser(m_pFloppy1, this, "Floppy 2 image", &sFloppyFilter);
            else if (pWindow_ == m_pBrowseFloppy2)
                new CFileBrowser(m_pFloppy2, this, "Floppy 1 image", &sFloppyFilter);
            else if (pWindow_ == m_pBrowseAtom)
                new CHDDProperties(m_pAtom, this, "Atom Hard Disk");
            else if (pWindow_ == m_pBrowseSDIDE)
                new CHDDProperties(m_pSDIDE, this, "SD-IDE Hard Disk");
            else if (pWindow_ == m_pBrowseYATBus)
                new CHDDProperties(m_pYATBus, this, "YATBus Hard Disk");
            {
            }
        }

    protected:
        CEditControl *m_pFloppy1, *m_pFloppy2, *m_pAtom, *m_pSDIDE, *m_pYATBus;
        CTextButton *m_pBrowseFloppy1, *m_pBrowseFloppy2, *m_pBrowseAtom, *m_pBrowseSDIDE, *m_pBrowseYATBus;
        CCheckBox* m_pTurboLoad;
        CTextButton *m_pOK, *m_pCancel;
};


class CParallelOptions : public CDialog
{
    public:
        CParallelOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 241, "Parallel Settings")
        {
            new CIconControl(this, 10, 10, &sPortIcon);
            new CFrameControl(this, 50, 17, 238, 91);
            new CTextControl(this, 60, 13, "Parallel Ports", YELLOW_8, BLUE_2);
            new CTextControl(this, 63, 33, "Devices connected to the parallel ports:");

            new CTextControl(this, 77, 57, "Port 1:");
            m_pPort1 = new CComboBox(this, 125, 54, "None|Printer|Mono DAC|Stereo DAC", 120);

            new CTextControl(this, 77, 82, "Port 2:");
            m_pPort2 = new CComboBox(this, 125, 79, "None|Printer|Mono DAC|Stereo DAC", 120);

            new CIconControl(this, 10, 113, &sPortIcon);
            new CFrameControl(this, 50, 120, 238, 79);

            new CTextControl(this, 60, 116, "Printer Device", YELLOW_8, BLUE_2);
            new CTextControl(this, 63, 136, "The following printer will be used for raw");
            new CTextControl(this, 63, 150, "SAM printer output:");

            m_pPrinter = new CComboBox(this, 63, 169, "<not currently supported>", 215);

            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

            // Set the initial state from the options
            m_pPort1->Select(GetOption(parallel1));
            m_pPort2->Select(GetOption(parallel2));

            // Update the state of the controls to reflect the current settings
            OnNotify(m_pPort1,0);
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {
            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pOK)
            {
                SetOption(parallel1, m_pPort1->GetSelected());
                SetOption(parallel2, m_pPort2->GetSelected());
                SetOption(printerdev, "");

                if (Changed(parallel1) || Changed(parallel2) || ChangedString(printerdev))
                    IO::InitParallel();

                Destroy();
            }
            else
            {
                bool fPrinter1 = m_pPort1->GetSelected() == 1, fPrinter2 = m_pPort2->GetSelected() == 1;
                m_pPrinter->Enable(fPrinter1 || fPrinter2);
                m_pPrinter->Enable(false);
            }
        }

    protected:
        CComboBox *m_pPort1, *m_pPort2, *m_pPrinter;
        CTextButton *m_pOK, *m_pCancel;
};


class CMiscOptions : public CDialog
{
    public:
        CMiscOptions (CWindow* pParent_)
            : CDialog(pParent_, 300, 241, "Misc Settings")
        {
            new CIconControl(this, 10, 15, &sHardwareIcon);
            new CFrameControl(this, 50, 17, 238, 77);
            new CTextControl(this, 60, 13, "Clocks", YELLOW_8, BLUE_2);
            m_pSambus = new CCheckBox(this, 63, 32, "SAMBUS Clock");
            m_pDallas = new CCheckBox(this, 63, 52, "DALLAS Clock");
            m_pClockSync = new CCheckBox(this, 63, 72, "Advance SAM time relative to real time.");

            new CFrameControl(this, 50, 109, 238, 102);
            new CTextControl(this, 60, 105, "Miscellaneous", YELLOW_8, BLUE_2);
            m_pPauseInactive = new CCheckBox(this, 63, 124, "Pause the emulation when inactive.");
            m_pDriveLights = new CCheckBox(this, 63, 144, "Show disk drive LEDs.");
            m_pStatus = new CCheckBox(this, 63, 164, "Display status messages.");
            new CTextControl(this, 63, 187, "Profiling stats:");
            m_pProfile = new CComboBox(this, 140, 184, "Disabled|Speed and frame rate|Details percentages|Detailed timings", 140);

            m_pOK = new CTextButton(this, m_nWidth - 117, m_nHeight-21, "OK", 50);
            m_pCancel = new CTextButton(this, m_nWidth - 62, m_nHeight-21, "Cancel", 50);

            // Set the initial state from the options
            m_pSambus->SetChecked(GetOption(sambusclock));
            m_pDallas->SetChecked(GetOption(dallasclock));
            m_pClockSync->SetChecked(GetOption(clocksync));

            m_pPauseInactive->SetChecked(GetOption(pauseinactive));
            m_pDriveLights->SetChecked(GetOption(drivelights) != 0);
            m_pStatus->SetChecked(GetOption(status));

            m_pProfile->Select(GetOption(profile));
        }

    public:
        void OnNotify (CWindow* pWindow_, int nParam_)
        {
            if (pWindow_ == m_pCancel)
                Destroy();
            else if (pWindow_ == m_pOK)
            {
                SetOption(sambusclock, m_pSambus->IsChecked());
                SetOption(dallasclock, m_pDallas->IsChecked());
                SetOption(clocksync, m_pClockSync->IsChecked());

                SetOption(pauseinactive, m_pPauseInactive->IsChecked());
                SetOption(drivelights, m_pDriveLights->IsChecked());
                SetOption(status, m_pStatus->IsChecked());

                SetOption(profile, m_pProfile->GetSelected());

                if (Changed(sambusclock) || Changed(dallasclock))
                    IO::InitClocks();

                Destroy();
            }
        }

    protected:
        CCheckBox *m_pSambus, *m_pDallas, *m_pClockSync;
        CCheckBox *m_pPauseInactive, *m_pDriveLights, *m_pStatus;
        CComboBox *m_pProfile;
        CTextButton *m_pOK, *m_pCancel;
};


void COptionsDialog::OnNotify (CWindow* pWindow_, int nParam_)
{
    if (pWindow_ == m_pClose)
        Destroy();
    else if (pWindow_ == m_pOptions)
    {
        const CListViewItem* pItem = m_pOptions->GetItem();
        if (pItem)
        {
            // Save the current options for change comparisons
            g_opts = Options::s_Options;

            if (!strcasecmp(pItem->m_pszLabel, "system"))
            {
                m_pStatus->SetText("Main/external memory configuration and ROM image paths.");
                if (nParam_) new CSystemOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "display"))
            {
                m_pStatus->SetText("Display settings for mode, depth, view size, etc.");
                if (nParam_) new CDisplayOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "sound"))
            {
                m_pStatus->SetText("Sound quality settings for SAA chip and beeper.");
                if (nParam_) new CSoundOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "midi"))
            {
                m_pStatus->SetText("MIDI settings for music and network.");
                if (nParam_) new CMidiOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "input"))
            {
                m_pStatus->SetText("Keyboard mapping and mouse settings.");
                if (nParam_) new CInputOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "drives"))
            {
                m_pStatus->SetText("Floppy disk drive configuration.");
                if (nParam_) new CDriveOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "disks"))
            {
                m_pStatus->SetText("Disks for floppy and hard disk drives.");
                if (nParam_) new CDiskOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "parallel"))
            {
                m_pStatus->SetText("Parallel port settings for printer and DACs).");
                if (nParam_) new CParallelOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "misc"))
            {
                m_pStatus->SetText("Clock settings and miscellaneous front-end options.");
                if (nParam_) new CMiscOptions(this);
            }
            else if (!strcasecmp(pItem->m_pszLabel, "about"))
            {
                m_pStatus->SetText("Display SimCoupe version number and credits.");
                if (nParam_) new CAboutDialog(this);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

char CImportDialog::s_szFile[MAX_PATH];
UINT CImportDialog::s_uAddr = 32768, CImportDialog::s_uPage, CImportDialog::s_uOffset;
bool CImportDialog::s_fUseBasic = true;

CImportDialog::CImportDialog (CWindow* pParent_)
    : CDialog(pParent_, 230, 165, "Import Data")
{
    new CTextControl(this, 10, 18,  "File:");
    m_pFile = new CEditControl(this, 35, 15, 160, s_szFile);
    m_pBrowse = new CTextButton(this, 200, 15, "...", 17);

    m_pFrame = new CFrameControl(this, 10, 47, 208, 88);
    new CTextControl(this, 20, 43, "Data", YELLOW_8, BLUE_2);

    m_pBasic = new CRadioButton(this, 33, 65, "BASIC Address:", 45);
    m_pPageOffset = new CRadioButton(this, 33, 90, "Page number:", 45);
    new CTextControl(this, 50, 110, "Page offset:", WHITE);

    m_pAddr = new CEditControl(this, 143, 63, 45, s_uAddr);
    m_pPage = new CEditControl(this, 143, 88, 20, s_uPage);
    m_pOffset = new CEditControl(this, 143, 108, 35, s_uOffset);

    int nX = (m_nWidth - (50+8+50)) / 2;
    m_pOK = new CTextButton(this, nX, m_nHeight-21, "OK", 50);
    m_pCancel = new CTextButton(this, nX+50+8, m_nHeight-21, "Cancel", 50);

    (s_fUseBasic ? m_pBasic : m_pPageOffset)->Select();
    OnNotify(m_pBasic, 0);
    OnNotify(s_fUseBasic ? m_pAddr : m_pPage, 0);
}

void CImportDialog::OnNotify (CWindow* pWindow_, int nParam_)
{
    static const FILEFILTER sImportFilter =
    {
        "Binary files (*.bin)|"
        "All Files",

        { ".bin", "" }
    };

    if (pWindow_ == m_pCancel)
        Destroy();
    else if (pWindow_ == m_pBrowse)
        new CFileBrowser(m_pFile, this, "Select File", &sImportFilter);
    else if (pWindow_ == m_pAddr)
    {
        // Fetch the modified address
        s_uAddr = m_pAddr->GetValue();

        // Calculate (and update) the new page and offset
        s_uPage = (s_uAddr/16384 - 1) & 0x1f;
        s_uOffset = s_uAddr & 0x3fff;
        m_pPage->SetValue(s_uPage);
        m_pOffset->SetValue(s_uOffset);
    }
    else if (pWindow_ == m_pPage || pWindow_ == m_pOffset)
    {
        // Fetch the modified page or offset
        s_uPage = m_pPage->GetValue() & 0x1f;
        s_uOffset = m_pOffset->GetValue();

        // Calculate (and update) the new address
        s_uAddr = ((s_uPage + 1) * 16384 + s_uOffset) % 0x84000;    // wrap at end of memory
        m_pAddr->SetValue(s_uAddr);

        // Normalise the internal page and offset from the address
        s_uPage = (s_uAddr/16384 - 1) & 0x1f;
        s_uOffset = s_uAddr & 0x3fff;
    }
    else if (pWindow_ == m_pBasic || pWindow_ == m_pPageOffset)
    {
        // Fetch the radio selection
        s_fUseBasic = m_pBasic->IsSelected();

        // Enable/disable the edit controls depending on the radio selection
        m_pAddr->Enable(s_fUseBasic);
        m_pPage->Enable(!s_fUseBasic);
        m_pOffset->Enable(!s_fUseBasic);
    }
    else if (pWindow_ == m_pOK || nParam_)
    {
        // Fetch/update the stored filename
        strncpy(s_szFile, m_pFile->GetText(), sizeof s_szFile);

        FILE* hFile;
        if (!s_szFile[0] || !(hFile = fopen(s_szFile, "rb")))
        {
            new CMessageBox(this, "Failed to open file for reading", "Error", mbWarning);
            return;
        }

        UINT uPage = (s_uAddr < 0x4000) ? ROM0 : s_uPage;
        UINT uOffset = s_uOffset, uLen = 0x7ffff, uRead = 0;

        // Loop reading chunk blocks into the relevant pages
        for (UINT uChunk ; uChunk = min(uLen, (0x4000 - uOffset)) ; uLen -= uChunk, uOffset = 0)
        {
            // Read directly into system memory
            uRead += fread(&apbPageWritePtrs[uPage][uOffset], 1, uChunk, hFile);

            // Stop reading if we've hit the end
            if (feof(hFile))
                break;

            // If the first block was in ROM0 or we've passed memory end, wrap to page 0
            if (++uPage >= N_PAGES_MAIN)
                uPage = 0;
        }

        fclose(hFile);
        Frame::SetStatus("%u bytes imported to %u", uRead, s_uAddr);
        Destroy();
    }
}


UINT CExportDialog::s_uLength = 16384;  // show 16K as the initial export length

CExportDialog::CExportDialog (CWindow* pParent_)
    : CImportDialog(pParent_)
{
    SetText("Export Data");

    // Enlarge the input dialog for the new controls
    int nOffset = 22;
    Offset(0, -nOffset/2);
    Inflate(0, nOffset);
    m_pFrame->Inflate(0, nOffset);
    m_pOK->Offset(0, nOffset);
    m_pCancel->Offset(0, nOffset);

    // Add the new controls for Export
    new CTextControl(this, 50, 135, "Length:", WHITE);
    m_pLength = new CEditControl(this, 143, 133, 45, s_uLength);
}


void CExportDialog::OnNotify (CWindow* pWindow_, int nParam_)
{
    if (pWindow_ == m_pOK || nParam_)
    {
        // Fetch/update the stored filename
        strncpy(s_szFile, m_pFile->GetText(), sizeof s_szFile);

        FILE* hFile;
        if (!s_szFile[0] || !(hFile = fopen(s_szFile, "wb")))
        {
            new CMessageBox(this, "Failed to open file for writing", "Error", mbWarning);
            return;
        }

        UINT uPage = (s_uAddr < 0x4000) ? ROM0 : s_uPage;
        UINT uOffset = s_uOffset, uLen = min(s_uLength, 0x84000), uWritten = 0;

        // Loop reading chunk blocks into the relevant pages
        for (UINT uChunk ; uChunk = min(uLen, (0x4000 - uOffset)) ; uLen -= uChunk, uWritten += uChunk, uOffset = 0)
        {
            // Write directly from system memory
            uWritten += fwrite(&apbPageReadPtrs[uPage][uOffset], 1, uChunk, hFile);

            if (ferror(hFile))
            {
                new CMessageBox(this, "Error writing to file (disk full?)", "Error", mbWarning);    
                fclose(hFile);
                return;
            }

            // If the first block was in ROM0 or we've passed memory end, wrap to page 0
            if (++uPage >= N_PAGES_MAIN)
                uPage = 0;
        }

        fclose(hFile);
        Frame::SetStatus("%u bytes exported from %u", uWritten, s_uAddr);
        Destroy();
    }

    // Pass to the base handler
    else
        CImportDialog::OnNotify(pWindow_, nParam_);
}
