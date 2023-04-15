
package zwift
//package com.oreilly.learningsparkexamples.mini.scala

import org.apache.spark._
import org.apache.spark.SparkContext._

object mainZwift{
    def main(args: Array[String]) {
      //define output location
val outputFile = "hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/resInvertedIndexSeq"
      val conf = new SparkConf().setAppName("invertedIndexSeq")
      // Create a Scala Spark Context.
      val sc = new SparkContext(conf)
      // Load our input data.

//input data
      val inputDic =  sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/dictionary.dic")
      val rdd0 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input0",1)
      val rdd1 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input1",1)
      val rdd2 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input2",1)
      val rdd3 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input3",1)
      val rdd4 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input4",1)
      val rdd5 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input5",1)


      val rdd = rdd0 ++ rdd1 ++ rdd2 ++ rdd3 ++ rdd4 ++ rdd5
    
      //pipe calling
      val scriptPath = "/home/ubuntu/spark-2.1.0-bin-hadoop2.7/invertedIndexSparkSeq/a.out"


      val pipeRDD = rdd.pipe(scriptPath).map(fstr).groupBy(_._1)
		.map(p => (p._1, p._2.map(_._2).toVector)).saveAsTextFile(outputFile)

    }

  
    def fstr(s: String) : (Int, Int) = {
      val split = s.split(" ")
        (split(0).toInt, split(1).toInt)
    }

}
