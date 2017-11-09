import glob
import random

paths = zip(sorted(glob.glob("outputs/*_data.txt")),sorted(glob.glob("outputs/*_labels.txt")))

random.shuffle(paths)

test_count = 2

print "Train:" + ";".join([x[0].split('/')[-1].split("_data")[0] for x in paths[:-test_count]])
train_data = open("final_data_train.txt", "w")
train_labels = open("final_labels_train.txt", "w")
for p in paths[:-test_count]:
	with open(p[0], "rb") as f:
		temp = f.read()
		train_data.write(temp)

	with open(p[1], "rb") as f:
		temp = f.read()
		train_labels.write(temp)

train_data.close()
train_labels.close()

print "Test:" + ";".join([x[0].split('/')[-1].split("_data")[0] for x in paths[-test_count:]])
test_data = open("final_data_test.txt", "w")
test_labels = open("final_labels_test.txt", "w")
for p in paths[-test_count:]:
	with open(p[0], "rb") as f:
		temp = f.read()
		test_data.write(temp)

	with open(p[1], "rb") as f:
		temp = f.read()
		test_labels.write(temp)

test_data.close()
test_labels.close()