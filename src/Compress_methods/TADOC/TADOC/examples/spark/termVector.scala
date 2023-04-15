
package zwift

import org.apache.spark._
import org.apache.spark.SparkContext._

object mainZwift{
    def main(args: Array[String]) {
val outputFile = "hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/resTermVectorSeq"
      val conf = new SparkConf().setAppName("termVectorSeq")
      val sc = new SparkContext(conf)

      val inputDic =  sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeq/dictionary.dic")
      val rdd0 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input0",1)
      val rdd1 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input1",1)
      val rdd2 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input2",1)
      val rdd3 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input3",1)
      val rdd4 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input4",1)
      val rdd5 =sc.textFile("hdfs://ec2-52-43-51-249.us-west-2.compute.amazonaws.com:9000/19_NSR_SparkSeqFile/fusion6Input5",1)

  
      val rdd = rdd0 ++ rdd1 ++ rdd2 ++ rdd3 ++ rdd4 ++ rdd5
    
      val scriptPath = "/home/ubuntu/spark-2.1.0-bin-hadoop2.7/termVectorSparkSeq/a.out"

      
      val pipeRDD = rdd.pipe(scriptPath).saveAsTextFile(outputFile)


    }

  
    def fstr(s: String) : (Int, Int) = {
      val split = s.split(" ")
        (split(0).toInt, split(1).toInt)
    }

}
