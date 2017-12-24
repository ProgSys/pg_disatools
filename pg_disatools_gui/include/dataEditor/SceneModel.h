/*
 * SceneModel.h
 *
 *  Created on: 23.11.2017
 *      Author: ProgSys
 */

#ifndef INCLUDE_FILEMANAGERD2_SCENEMODEL_H_
#define INCLUDE_FILEMANAGERD2_SCENEMODEL_H_

#include <QAbstractItemModel>
#include <string>
#include <vector>
#include <map>

enum TreeTypes: int{
	Type_A = 0,
	Type_B,
	Type_C,
	TYPE_MAX,
};

inline std::string toString(TreeTypes type){
	switch (type) {
		case Type_A: return "Type_A";break;
		case Type_B: return "Type_B";break;
		case Type_C: return "Type_C";break;
		default: return "Type_?"; break;
	}
}

struct Item{
	std::string name;
	TreeTypes type;
};

class SceneModel: public QAbstractItemModel {
	Q_OBJECT
public:
	SceneModel(QObject *parent = nullptr);
	virtual ~SceneModel();

	void add(TreeTypes type, const QString& name);
	void remove(TreeTypes type, const QString& name);

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    std::vector<std::map<std::string, Item>> mItems;
};

#endif /* INCLUDE_FILEMANAGERD2_SCENEMODEL_H_ */
