/*  YUView - YUV player with advanced analytics toolset
*   Copyright (C) 2015  Institut für Nachrichtentechnik
*                       RWTH Aachen University, GERMANY
*
*   YUView is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   YUView is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with YUView.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLAYLISTITEMHEVCFILE_H
#define PLAYLISTITEMHEVCFILE_H

#include "de265Decoder.h"
#include "playlistItemWithVideo.h"
#include "statisticHandler.h"
#include "videoHandlerYUV.h"

class videoHandler;

class playlistItemHEVCFile : public playlistItemWithVideo
{
  Q_OBJECT

public:

  /* The default constructor requires the user to set a name that will be displayed in the treeWidget and
   * provide a pointer to the widget stack for the properties panels. The constructor will then call
   * addPropertiesWidget to add the custom properties panel.
  */
  playlistItemHEVCFile(const QString &fileName);

  // Save the HEVC file element to the given XML structure.
  virtual void savePlaylist(QDomElement &root, const QDir &playlistDir) const Q_DECL_OVERRIDE;
  // Create a new playlistItemHEVCFile from the playlist file entry. Return nullptr if parsing failed.
  static playlistItemHEVCFile *newplaylistItemHEVCFile(const QDomElementYUView &root, const QString &playlistFilePath);

  // Return the info title and info list to be shown in the fileInfo groupBox.
  virtual infoData getInfo() const Q_DECL_OVERRIDE;
  virtual void infoListButtonPressed(int buttonID);

  virtual QString getPropertiesTitle() const Q_DECL_OVERRIDE { return "HEVC File Properties"; }

  // Draw the item using the given painter and zoom factor. If the item is indexed by frame, the given frame index will be drawn. If the
  // item is not indexed by frame, the parameter frameIdx is ignored.
  virtual void drawItem(QPainter *painter, int frameIdx, double zoomFactor, bool drawRawData) Q_DECL_OVERRIDE;

  // Return the source (YUV and statistics) values under the given pixel position.
  virtual ValuePairListSets getPixelValues(const QPoint &pixelPos, int frameIdx) Q_DECL_OVERRIDE;

  // If you want your item to be droppable onto a difference object, return true here and return a valid video handler.
  virtual bool canBeUsedInDifference() const Q_DECL_OVERRIDE { return true; }

  // Override from playlistItemIndexed. The annexBFile handler can tell us how many POSs there are.
  virtual indexRange getStartEndFrameLimits() const Q_DECL_OVERRIDE { return indexRange(0, loadingDecoder.getNumberPOCs()-1); }

  // Add the file type filters and the extensions of files that we can load.
  static void getSupportedFileExtensions(QStringList &allExtensions, QStringList &filters);

  // ----- Detection of source/file change events -----
  virtual bool isSourceChanged()        Q_DECL_OVERRIDE { return loadingDecoder.isFileChanged(); }
  virtual void reloadItemSource()       Q_DECL_OVERRIDE;
  virtual void updateFileWatchSetting() Q_DECL_OVERRIDE { loadingDecoder.updateFileWatchSetting(); }

  // Do we need to load the given frame first?
  virtual itemLoadingState needsLoading(int frameIdx, bool loadRawData) Q_DECL_OVERRIDE;
  // Load the frame in the video item. Emit signalItemChanged(true,false) when done.
  virtual void loadFrame(int frameIdx, bool playing, bool loadRawData) Q_DECL_OVERRIDE;
  // Is an image currently being loaded?
  virtual bool isLoading() const Q_DECL_OVERRIDE { return isFrameLoading; }
  virtual bool isLoadingDoubleBuffer() const Q_DECL_OVERRIDE { return isFrameLoadingDoubleBuffer; }

  // For HEVC items, a mutex must be locked when caching a frame.
  void cacheFrame(int idx);

public slots:
  // Load the YUV data for the given frame index from file. This slot is called by the videoHandlerYUV if the frame that is
  // requested to be drawn has not been loaded yet.
  virtual void loadYUVData(int frameIdx, bool forceDecodingNow);

  // The statistic with the given frameIdx/typeIdx could not be found in the cache. Load it.
  virtual void loadStatisticToCache(int frameIdx, int typeIdx);

protected:
  virtual void createPropertiesWidget() Q_DECL_OVERRIDE;

private:

  // We allocate two decoder: One for loading images in the foreground and one for caching in the background.
  // This is better if random access and linear decoding (caching) is performed at the same time.
  de265Decoder loadingDecoder;
  de265Decoder cachingDecoder;

  // Is the loadFrame function currently loading?
  bool isFrameLoading;
  bool isFrameLoadingDoubleBuffer;

  // Only cache one frame at a time. Caching should also always be done in display order of the frames.
  // TODO: Could we somehow make shure that caching is always performed in display order?
  QMutex cachingMutex;

  // The statistics source
  statisticHandler statSource;

  // fill the list of statistic types that we can provide
  void fillStatisticList();

  // Get the statistics from the frame and put them into the local cache for the current frame
  void cacheStatistics(const de265_image *img, int iPOC);

private slots:
  void updateStatSource(bool bRedraw) { emit signalItemChanged(bRedraw, false); }

};

#endif // PLAYLISTITEMHEVCFILE_H
