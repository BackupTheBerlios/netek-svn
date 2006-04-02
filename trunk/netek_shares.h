#ifndef __NETEK_SHARES_H__
#define __NETEK_SHARES_H__

#include <QtGui>

namespace neteK {

class Share;

class Shares: public QObject {
		Q_OBJECT;

		QList<QPointer<Share> > m_shares;
		void initShare(Share *);

	public slots:
		int createShareWithSettings(QString path = "");
		int createShareResolvePath(QString path);
		bool deleteShareWithQuestion(int i);
		void settingsChanged();
		
	signals:
		void changed();

	public:
		Shares();

		Share *share(int i) const;
		int shares() const;
		
		static bool resolvePathForShare(QString file, QString &path);

};

}

#endif
