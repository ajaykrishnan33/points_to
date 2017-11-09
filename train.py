import pprint
import numpy as np

from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split

from sklearn.neural_network import MLPClassifier
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC
from sklearn.gaussian_process import GaussianProcessClassifier
from sklearn.gaussian_process.kernels import RBF
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.discriminant_analysis import QuadraticDiscriminantAnalysis

from sklearn.metrics import confusion_matrix

def load_data():
	X_train = np.loadtxt("final_data_train.txt")
	y_train = np.loadtxt("final_labels_train.txt")

	X_test = np.loadtxt("final_data_test.txt")
	y_test = np.loadtxt("final_labels_test.txt")

	return (X_train,X_test, y_train, y_test)

def train():
	X_train, X_test, y_train, y_test = load_data()

	X_train = StandardScaler().fit_transform(X_train)
	X_test = StandardScaler().fit_transform(X_test)
	# X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.1, random_state=42)

	names = [
		"Nearest Neighbors", "Linear SVM", "RBF SVM", 
		# "Gaussian Process", "Decision Tree", "Random Forest", "Neural Net", "AdaBoost",
  #       "Naive Bayes", "QDA"
    ]

	classifiers = [
	    KNeighborsClassifier(3),
	    SVC(kernel="linear", C=0.025),
	    SVC(gamma=2, C=1),
	    # GaussianProcessClassifier(1.0 * RBF(1.0)),
	    # DecisionTreeClassifier(max_depth=5),
	    # RandomForestClassifier(max_depth=5, n_estimators=10, max_features=1),
	    # MLPClassifier(alpha=1),
	    # AdaBoostClassifier(),
	    # GaussianNB(),
	    # QuadraticDiscriminantAnalysis()
    ]
	
	for name, clf in zip(names, classifiers):
		clf.fit(X_train, y_train)
		score = clf.score(X_test, y_test)
		print name + ":" + str(score)
		y_pred = clf.predict(X_test)
		print confusion_matrix(y_test, y_pred)

train()